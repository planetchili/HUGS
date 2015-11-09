#include <assert.h>
#include "Sound.h"

SoundSystem& SoundSystem::Get()
{
	static SoundSystem instance;
	return instance;
}

SoundSystem::SoundSystem()
{
	LoadLibrary( L"setupapi.dll" );
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 176400;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
	format.cbSize = 0;
	XAudio2Create( &pEngine );
	pEngine->CreateMasteringVoice( &pMaster );
	for( int i = 0; i < nChannels; i++ )
	{
		idleChannelPtrs.push_back( std::make_unique<Channel>( *this ) );
	}
}

// chili doesn't trust that this callback won't fuck you up (multithreaded race condition)
void SoundSystem::Channel::VoiceCallback::OnBufferEnd( void* pBufferContext )
{
	Channel& chan = *(Channel*)pBufferContext;
	chan.pSound->RemoveChannel( chan );
	chan.pSound = nullptr;
	SoundSystem::Get().DeactivateChannel( chan );
}

void SoundSystem::Channel::Play( Sound& s )
{
	assert( pSource && !pSound );
	s.AddChannel( *this );
	pSound = &s;
	xaBuffer.pAudioData = s.pData.get();
	xaBuffer.AudioBytes = s.nBytes;
	pSource->SubmitSourceBuffer( &xaBuffer,nullptr );
	pSource->Start();
}
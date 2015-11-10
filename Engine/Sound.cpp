#include <assert.h>
#include "Sound.h"

SoundSystem& SoundSystem::Get()
{
	static SoundSystem instance;
	return instance;
}

SoundSystem::SoundSystem()
{
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

void SoundSystem::Channel::VoiceCallback::OnBufferEnd( void* pBufferContext )
{
	Channel& chan = *(Channel*)pBufferContext;
	chan.Stop();
	chan.pSound->RemoveChannel( chan );
	chan.pSound = nullptr;
	SoundSystem::Get().DeactivateChannel( chan );
}

void SoundSystem::Channel::PlaySoundBuffer( Sound& s )
{
	assert( pSource && !pSound );
	s.AddChannel( *this );
	// callback thread not running yet, so no sync necessary for pSound
	pSound = &s;
	xaBuffer.pAudioData = s.pData.get();
	xaBuffer.AudioBytes = s.nBytes;
	pSource->SubmitSourceBuffer( &xaBuffer,nullptr );
	pSource->Start();
}
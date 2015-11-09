#include <assert.h>
#include "Sound.h"

SoundSystem& SoundSystem::Get()
{
	static SoundSystem instance;
	return instance;
}

SoundSystem::~SoundSystem()
{
	for( auto& a : activeChannelPtrs )
	{
		a.reset();
	}
	for( auto& a : idleChannelPtrs )
	{
		a.reset();
	}
	if( pEngine )
	{
		pEngine->Release();
		pEngine = nullptr;
	}
	pMaster = nullptr;
	CoUninitialize();
}

SoundSystem::SoundSystem()
{
	CoInitializeEx( NULL,COINIT_MULTITHREADED );
	format.cbSize = 24932;
	format.wFormatTag = 1;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.nAvgBytesPerSec = 176400;
	format.nBlockAlign = 4;
	format.wBitsPerSample = 16;
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
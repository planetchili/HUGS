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

void SoundSystem::Channel::PlaySoundBuffer( Sound& s,float freqMod,float vol )
{
	assert( pSource && !pSound );
	s.AddChannel( *this );
	// callback thread not running yet, so no sync necessary for pSound
	pSound = &s;
	xaBuffer.pAudioData = s.pData.get();
	xaBuffer.AudioBytes = s.nBytes;
	if( s.loopStart != 0xFFFFFFFFu )
	{
		xaBuffer.LoopBegin = s.loopStart;
		xaBuffer.LoopLength = s.loopEnd - s.loopStart;
		xaBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else
	{
		xaBuffer.LoopCount = 0u;
	}
	pSource->SubmitSourceBuffer( &xaBuffer,nullptr );
	pSource->SetFrequencyRatio( freqMod );
	pSource->SetVolume( vol );
	pSource->Start();
}
#pragma once
#include "Sound.h"
#include <random>

class SoundEffect
{
public:
	SoundEffect( const std::wstring& fileName,float stdDev,unsigned int seed )
		:
		rng( seed ),
		freqDist( 1.0f,stdDev ),
		sound( fileName )
	{}
	void Play()
	{
		sound.Play( freqDist( rng ) );
	}
private:
	std::mt19937 rng;
	std::normal_distribution<float> freqDist;
	Sound sound;
};
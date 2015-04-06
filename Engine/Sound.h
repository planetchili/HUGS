/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	Sound.h																				  *
 *	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
 *  Based on code obtained from http://www.rastertek.com								  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>

class DSound;

class Sound
{
	friend DSound;
public:
	Sound( const Sound& base );
	Sound();
	~Sound();
	const Sound& operator=( const Sound& rhs );
	void Play( int attenuation = DSBVOLUME_MAX );
private:
	Sound( IDirectSoundBuffer8* pSecondaryBuffer );
private:
	IDirectSoundBuffer8* pBuffer;
};

class DSound
{
private:
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};
public:
	DSound( HWND hWnd );
	~DSound();
	Sound CreateSound( char* wavFileName );
private:
	DSound();
private:	
	IDirectSound8* pDirectSound;
	IDirectSoundBuffer* pPrimaryBuffer;
};

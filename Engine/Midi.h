#pragma once
#include <string>
#include <mutex>
#include <condition_variable>
#include "UIDManager.h"

class MidiSong
{
public:
	MidiSong( const std::wstring path,float start_sec,float end_sec,bool looping = true );
	void Play();
	void Stop();
	~MidiSong();
private:
	std::wstring GetAlias() const;
private:
	static UniqueIDManager<unsigned int> idMan;
	std::mutex mutex;
	std::condition_variable cv;
	unsigned int id;
	// this doubles as signal to ctor-caller that init is done (false=init done)
	bool isPlaying = true;
	// this doubles as signal to dtor-caller that dying is done (false=dying done)
	bool isDying = false;
	bool looping;
	unsigned int startMilliSeconds;
	unsigned int endMilliSeconds;
};
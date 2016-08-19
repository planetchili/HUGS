#include "Midi.h"
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <thread>
#include <chrono>
#pragma comment(lib, "winmm.lib" )

UniqueIDManager<unsigned int> MidiSong::idMan;

MidiSong::MidiSong( const std::wstring path,float start,float end,bool looping )
	:
	id( idMan.Issue() ),
	startMilliSeconds( unsigned int( start * 1000.0f ) ),
	endMilliSeconds( unsigned int( end * 1000.0f ) ),
	looping( looping )
{
	// create thread to execute lambda function
	std::thread( [this,path,end]()
	{
		// make sure the ctor-calling thread is waiting for notification
		std::unique_lock<std::mutex> threadLock( mutex );

		// init command strings
		const std::wstring openCmd = std::wstring( L"open " ) + path + 
			std::wstring( L" type sequencer alias " ) + GetAlias();
		const std::wstring setFormatCmd = std::wstring( L"set " ) + GetAlias() +
			std::wstring( L" time format milliseconds" );
		const std::wstring seekStartCmd = std::wstring( L"seek " ) + GetAlias() +
			std::wstring( L" to start" );
		const std::wstring seekLoopCmd = std::wstring( L"seek " ) + GetAlias() +
			std::wstring( L" to " ) + std::to_wstring( startMilliSeconds );
		const std::wstring playCmd = std::wstring( L"play " ) + GetAlias();
		const std::wstring stopCmd = std::wstring( L"stop " ) + GetAlias();
		const std::wstring closeCmd = std::wstring( L"close " ) + GetAlias();

		// open midi file
		mciSendString( openCmd.c_str(),nullptr,0u,nullptr );
		// set time format
		mciSendString( setFormatCmd.c_str(),nullptr,0u,nullptr );
		
		// set isPlaying = false to signal ctor-calling thread that init is done
		isPlaying = false;
		// wake up ctor-calling thread (init done)
		cv.notify_all();

		// check for death command
		while( !isDying )
		{
			// set loop duration for playing from beginning to end of loop region
			unsigned int duration = endMilliSeconds;
			// wait for command
			// this is one of the few places where we don't hold the mutex
			// ctor-calling thread (already woken up) can now continue
			cv.wait( threadLock,[this](){ return isDying || isPlaying;} );

			// loop check if stop command issued
			while( isPlaying )
			{
				// play that number maestro!
				mciSendString( playCmd.c_str(),nullptr,0u,nullptr );
				// wait until a) it's time to loop or b) we got a command
				// c) spurious wakeup is a PITA so we won't worry about it here ;)
				// this is really the only other place where we don't hold the mutex
				cv.wait_for( threadLock,
					std::chrono::milliseconds( duration ) );

				// seek to start of loop region
				mciSendString( seekLoopCmd.c_str(),nullptr,0u,nullptr );
				// set loop duration for playing from start of loop region to end of loop region
				duration = endMilliSeconds - startMilliSeconds;
				// don't loop if we're not a looper
				isPlaying = isPlaying && this->looping;
			}
			// stop command issued so stop the song from playing
			mciSendString( stopCmd.c_str(),nullptr,0u,nullptr );
			// stopped song so seek back to very beginning
			mciSendString( seekStartCmd.c_str(),nullptr,0u,nullptr );
		}
		// close mci file
		mciSendString( closeCmd.c_str(),nullptr,0u,nullptr );
		// acknowledge death command by setting isDying = false
		isDying = false;
		// unblock main thread which should be waiting for death
		cv.notify_all();
	} ).detach();	// detach created thread from std::thread object 
					// before destructor is called (so that apocolyse doesn't happen)
}

void MidiSong::Play()
{
	if( !isPlaying )
	{
		{
			std::lock_guard<std::mutex> lock( mutex );
			isPlaying = true;
		}
		cv.notify_all();
	}
}

void MidiSong::Stop()
{
	if( isPlaying )
	{
		{
			std::lock_guard<std::mutex> lock( mutex );
			isPlaying = false;
		}
		cv.notify_all();
	}
}

std::wstring MidiSong::GetAlias() const
{
	return std::wstring( L"ChiliMidi_" ) + std::to_wstring( id );
}

MidiSong::~MidiSong()
{
	std::unique_lock<std::mutex> lock( mutex );
	isPlaying = false;
	isDying = true;
	// wake up loop thread
	cv.notify_all();
	// unblock loop thread and wait on condition variable for loop thread death signal
	// here isDying == false means that the acknowledge the signal and has died
	cv.wait( lock,[this](){ return !isDying;} );
	// release the UID
	idMan.Release( id );
}
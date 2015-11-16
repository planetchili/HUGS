#pragma once
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <memory>
#include <vector>
#include <string>
#include "ChiliMath.h"
#include "Vec2.h"
#include <array>
#include <assert.h>
#include <queue>
#include <set>
#include <wrl\client.h>


class Gamepad
{
public:
	enum StickType : int
	{
		Dpad = 0,
		Analog1 = 1,
		Analog2 = 2
	};
	class Event
	{
	public:
		enum Type
		{
			Button = 0,
			Axis = 1,
			Stick = 2,
			Invalid = 3
		};
	public:
		// stick event
		Event( int index,Vec2 pos )
			:
			index( index ),
			pos( pos ),
			type( Stick )
		{}
		// axis event
		Event( float val )
			:
			index( -1 ),
			val( val ),
			type( Axis )
		{}
		// button event
		Event( int index,bool pressed )
			:
			index( index ),
			pressed( pressed ),
			type( Button )
		{}
		Event()
			:
			type( Invalid ),
			index( -1 ),
			pressed( false )
		{}
		float GetAxisVal() const
		{
			assert( type == Axis );
			return val;
		}
		int GetIndex() const
		{
			assert( type == Button || type == Stick );
			return index;
		}
		bool IsPressed() const
		{
			assert( type == Button );
			return pressed;
		}
		Vec2 GetStickPos() const
		{
			assert( type == Stick );
			return pos;
		}
		bool IsValid() const
		{
			return type != Invalid;
		}
		Type GetType() const
		{
			return type;
		}
		unsigned int GetHash() const
		{
			return (index << 2) | type;
		}
	private:
		Type type;
		int index;
		union
		{
			struct
			{
				Vec2 pos;
			};
			float val;
			bool pressed;
		};
	};
	class Filter
	{
		friend Gamepad;
	public:
		Filter( std::initializer_list<unsigned int> keys )
			:
			filterEvents( keys )
		{}
		Filter()
		{}
		bool Empty() const
		{
			return buffer.empty();
		}
		void AddCondition( const Event& e )
		{
			filterEvents.insert( e.GetHash() );
		}
		Event GetEvent()
		{
			const Event e( buffer.front() );
			buffer.pop_front();
			return e;
		}
	private:
		std::set<unsigned int> filterEvents;
		std::deque<Event> buffer;
	};
public:
	Gamepad( IDirectInput8W* pInput,HWND hWnd,const GUID& guid )
	{
		pInput->CreateDevice( guid,&pDev,nullptr ); 
		pDev->SetDataFormat( &c_dfDIJoystick2 );
		{
			DIPROPDWORD prop;
			prop.diph.dwHeaderSize = sizeof( prop.diph );
			prop.diph.dwSize = sizeof( prop );
			prop.diph.dwHow = DIPH_DEVICE;
			prop.diph.dwObj = 0;
			prop.dwData = 256;
			pDev->SetProperty( DIPROP_BUFFERSIZE,&prop.diph );
		}
		pDev->EnumObjects( EnumObjectsCallback,this,DIDFT_ALL );
		pDev->SetCooperativeLevel( hWnd,DISCL_BACKGROUND | DISCL_EXCLUSIVE );
	}
	~Gamepad()
	{
		if( pDev )
		{
			pDev->Unacquire();
		}
	}
	Vec2 GetStickData( int index = 0 ) const
	{
		// preset for 1 pov control (0) and 2 analog sticks (1 and 2)
		assert( index >= Dpad && index <= Analog2 );
		switch( index )
		{
		case Dpad: // pov values set for d-pad / might not work for other objects
			switch( state.rgdwPOV[0] )
			{
			case 0:
				return { 0.0f,-1000.f };
			case 4500:
				return { 1000.0f,-1000.f };
			case 9000:
				return { 1000.0f,0.f };
			case 13500:
				return { 1000.0f,1000.f };
			case 18000:
				return { 0.0f,1000.f };
			case 22500:
				return { -1000.0,1000.f };
			case 27000:
				return { -1000.0,0.f };
			case 31500:
				return { -1000.0,-1000.f };
			default:
				return { 0.0f,0.0f };
			}
		case Analog1:
			return { float( state.lX ),float( state.lY ) };
		case Analog2:
			return { float( state.lRx ),float( state.lRy ) };
		default:
			return { 0.0f,0.0f };
		}
	}
	float GetAxisData() const
	{
		return float( state.lZ );
	}
	bool GetButtonData( int index ) const
	{
		assert( index > 0 && index <= 127 );
		return (state.rgbButtons[index] & 0x80) == 0x80;
	}
	void Update()
	{
		pDev->Acquire();
		pDev->Poll();
		pDev->GetDeviceState( sizeof( state ),&state );

		DWORD nItems = 20;
		std::array<DIDEVICEOBJECTDATA,20> data;
		pDev->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ),data.data(),&nItems,0 );
		for( unsigned int i = 0; i < nItems; i++ )
		{
			if( data[i].dwOfs >= offsetof( DIJOYSTATE2,rgbButtons[0] ) &&
				data[i].dwOfs <= offsetof( DIJOYSTATE2,rgbButtons[127] ) )
			{
				events.emplace_back( data[i].dwOfs - offsetof( DIJOYSTATE2,rgbButtons[0] ),
					(data[i].dwData & 0x80) == 0x80 );
			}
			else if( data[i].dwOfs == offsetof( DIJOYSTATE2,rgdwPOV ) )
			{
				Vec2 pos;
				switch( data[i].dwData )
				{
				case 0:
					pos = { 0.0f,-1000.f };
					break;
				case 4500:
					pos = { 1000.0f,-1000.f };
					break;
				case 9000:
					pos = { 1000.0f,0.f };
					break;
				case 13500:
					pos = { 1000.0f,1000.f };
					break;
				case 18000:
					pos = { 0.0f,1000.f };
					break;
				case 22500:
					pos = { -1000.0,1000.f };
					break;
				case 27000:
					pos = { -1000.0,0.f };
					break;
				case 31500:
					pos = { -1000.0,-1000.f };
					break;
				default:
					pos = { 0.0f,0.0f };
				}
				events.emplace_back( Dpad,pos );
			}
			else if( data[i].dwOfs == offsetof( DIJOYSTATE2,lX ) ||
				data[i].dwOfs == offsetof( DIJOYSTATE2,lY ) )
			{
				events.emplace_back( Analog1,Vec2 { float( state.lX ),float( state.lY ) } );
			}
			else if( data[i].dwOfs == offsetof( DIJOYSTATE2,lRx ) ||
				data[i].dwOfs == offsetof( DIJOYSTATE2,lRy ) )
			{
				events.emplace_back( Analog2,Vec2 { float( state.lRx ),float( state.lRy ) } );
			}
			else if( data[i].dwOfs == offsetof( DIJOYSTATE2,lZ ) )
			{
				events.emplace_back( float( data[i].dwData ) );
			}
			while( events.size() > maxEvents )
			{
				events.pop_back();
			}
		}
	}
	Event ReadEvent()
	{
		Event e = events.front();
		events.pop_back();
		return e;
	}
	Event PeekEvent() const
	{
		return events.front();
	}
	void ExtractEvents( Filter& f )
	{
		const auto i = std::remove_if( events.begin(),events.end(),
			[&f]( const Event& e ) -> bool
		{
			const auto end = f.filterEvents.end();
			if( f.filterEvents.find( e.GetHash() ) != end )
			{
				f.buffer.push_back( e );
				return true;
			}
			return false;
		} );
		events.erase( i,events.end() );
	}
	bool IsEmpty() const
	{
		return events.empty();
	}
private:
	static BOOL CALLBACK
		EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext )
	{
		Gamepad* pPad = reinterpret_cast<Gamepad*>( pContext );
		if( pdidoi->dwType & DIDFT_AXIS )
		{
			DIPROPRANGE diprg;
			diprg.diph.dwSize = sizeof( DIPROPRANGE );
			diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			diprg.diph.dwHow = DIPH_BYID;
			diprg.diph.dwObj = pdidoi->dwType;
			diprg.lMin = -1000;
			diprg.lMax = +1000;

			pPad->pDev->SetProperty( DIPROP_RANGE,&diprg.diph );
			
			DIPROPDWORD didw;
			didw.diph.dwSize = sizeof( DIPROPDWORD );
			didw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			didw.diph.dwHow = DIPH_BYID;
			didw.diph.dwObj = pdidoi->dwType;
			didw.dwData = 100;
			
			pPad->pDev->SetProperty( DIPROP_DEADZONE,&didw.diph );
		}

		return DIENUM_CONTINUE;
	}
private:
	const unsigned int maxEvents = 64;
	DIJOYSTATE2 state;
	Microsoft::WRL::ComPtr<IDirectInputDevice8W> pDev;
	std::deque<Event> events;
};

class DirectInput
{
public:
	DirectInput( HWND hWnd )
		:
		hWnd( hWnd )
	{
		DirectInput8Create( GetModuleHandle( NULL ),DIRECTINPUT_VERSION,
			IID_IDirectInput8,(VOID**)&pInput,NULL );

		pInput->EnumDevices( DI8DEVCLASS_GAMECTRL,DeviceEnumCallback,
			(VOID*)this,DIEDFL_ATTACHEDONLY );
	}
	Gamepad& GetPad()
	{
		return *pPad;
	}
private:
	static BOOL CALLBACK
		DeviceEnumCallback( const DIDEVICEINSTANCE* instance,VOID* context )
	{
		DirectInput* pInput = static_cast<DirectInput*>( context );
		pInput->pPad = std::make_unique<Gamepad>( pInput->pInput.Get(),pInput->hWnd,instance->guidInstance );
		return DIENUM_STOP;
	}
private:
	const HWND hWnd;
	std::unique_ptr<Gamepad> pPad;
	Microsoft::WRL::ComPtr<IDirectInput8W> pInput;
};
/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	Keyboard.cpp																		  *
 *	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
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
#include "Keyboard.h"

KeyboardClient::KeyboardClient( KeyboardServer& kServer )
	: server( kServer )
{}

bool KeyboardClient::KeyIsPressed( unsigned char keycode ) const
{
	return server.keystates[ keycode ];
}

KeyEvent KeyboardClient::ReadKey()
{
	if( server.keybuffer.size() > 0 )
	{
		KeyEvent e = server.keybuffer.front();
		server.keybuffer.pop_front();
		return e;
	}
	else
	{
		return KeyEvent( KeyEvent::Invalid,0 );
	}
}

KeyEvent KeyboardClient::PeekKey() const
{	
	if( server.keybuffer.size() > 0 )
	{
		return server.keybuffer.front();
	}
	else
	{
		return KeyEvent( KeyEvent::Invalid,0 );
	}
}

bool KeyboardClient::KeyEmpty() const
{
	return server.keybuffer.empty();
}

unsigned char KeyboardClient::ReadChar()
{
	if( server.charbuffer.size() > 0 )
	{
		unsigned char charcode = server.charbuffer.front();
		server.charbuffer.pop_front();
		return charcode;
	}
	else
	{
		return 0;
	}
}

void KeyboardClient::ExtractEvents( KeyboardFilter& f )
{
	const auto i = std::remove_if( server.keybuffer.begin(),server.keybuffer.end(),
		[&f]( const KeyEvent& e ) -> bool
	{
		const auto end = f.filterKeys.end();
		if( f.filterKeys.find( e.GetCode() ) != end )
		{
			f.buffer.push_back( e );
			return true;
		}
		return false;
	} );
	server.keybuffer.erase( i,server.keybuffer.end() );
}

unsigned char KeyboardClient::PeekChar() const
{
	if( server.charbuffer.size() > 0 )
	{
		return server.charbuffer.front();
	}
	else
	{
		return 0;
	}
}

bool KeyboardClient::CharEmpty() const
{
	return server.charbuffer.empty();
}

void KeyboardClient::FlushKeyBuffer()
{
	while( !server.keybuffer.empty() )
	{
		server.keybuffer.pop_front();
	}
}

void KeyboardClient::FlushCharBuffer()
{
	while( !server.charbuffer.empty() )
	{
		server.charbuffer.pop_front();
	}
}

void KeyboardClient::FlushBuffers()
{
	FlushKeyBuffer();
	FlushCharBuffer();
}

KeyboardServer::KeyboardServer()
{
	for( int x = 0; x < nKeys; x++ )
	{
		keystates[ x ] = false;
	}
}

void KeyboardServer::OnKeyPressed( unsigned char keycode )
{
	keystates[ keycode ] = true;
	
	keybuffer.push_back( KeyEvent( KeyEvent::Press,keycode ) );
	if( keybuffer.size() > bufferSize )
	{
		keybuffer.pop_front();
	}
}

void KeyboardServer::OnKeyReleased( unsigned char keycode )
{
	keystates[ keycode ] = false;
	keybuffer.push_back( KeyEvent( KeyEvent::Release,keycode ) );
	if( keybuffer.size() > bufferSize )
	{
		keybuffer.pop_front();
	}
}

void KeyboardServer::OnChar( unsigned char character )
{
	charbuffer.push_back( character );
	if( charbuffer.size() > bufferSize )
	{
		charbuffer.pop_front();
	}
}


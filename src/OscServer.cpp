/*
 Copyright (C) 2012-2015 Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#define _WINSOCKAPI_    // stops windows.h including winsock.h

#include "cinder/app/App.h"

#include "boost/lexical_cast.hpp"

#include "OscServer.h"

using namespace std;

namespace osc {

Server::Server( int port, Proto proto /* = PROTO_UDP */ )
{
	string portStr = boost::lexical_cast< string >( port );
	mThread = lo_server_thread_new_with_proto( ( port == PORT_ANY ) ? NULL : portStr.c_str(), proto, errorHandler );
	if ( port == PORT_ANY )
	{
		mPort = lo_server_thread_get_port( mThread );
	}
	else
	{
		mPort = port;
	}

	lo_server_thread_start( mThread );
}

Server::~Server()
{
	lo_server_thread_free( mThread );
}

void Server::unregisterOscReceived( uint32_t callbackId )
{
	string path = mCallbackSpecs[ callbackId ].first;
	string typeSpec = mCallbackSpecs[ callbackId ].second;
	const char *pathPtr = path.empty() ? NULL : path.c_str();
	const char *typeSpecPtr = typeSpec.empty() ? NULL : typeSpec.c_str();

	lo_server_thread_del_method( mThread, pathPtr, typeSpecPtr );
	mCallbackSpecs.erase( callbackId );
}

void Server::errorHandler( int num, const char *msg, const char *path )
{
  // Here was a crash (invalid data)
	//ci::app::console() << "liblo server error " << num << " in path " << path << ": " << msg << endl;

  // So I had to write it myself
  
  const string errorString = [&]()
  {
    const string prefix = "liblo server error: ";

    map <int, string> loErrorToErrorString = { { 9901, "LO_ENOPATH" },
                                               { 9902, "LO_ENOTYPE" },     
                                               { 9903, "LO_UNKNOWNPROTO" },
                                               { 9904, "LO_NOPORT" },
                                               { 9905, "LO_TOOBIG" },
                                               { 9906, "LO_INT_ERR" },
                                               { 9907, "LO_EALLOC" },
                                               { 9908, "LO_EINVALIDPATH" },
                                               { 9909, "LO_EINVALIDTYPE" },
                                               { 9910, "LO_EBADTYPE" },
                                               { 9911, "LO_ESIZE" },
                                               { 9912, "LO_EINVALIDARG" },
                                               { 9913, "LO_ETERM" },
                                               { 9914, "LO_EPAD" },
                                               { 9915, "LO_EINVALIDBUND" },
                                               { 9916, "LO_EINVALIDTIME" } };

    auto it = loErrorToErrorString.find(num);
    if (it != loErrorToErrorString.end())
    {
      return prefix + it->second;
    }
    return prefix + "unknown (code " + to_string(num) + ")";
  }();

  //cerr << "osc::Server::errorHandler: " << errorString << endl;
  throw ServerExc(errorString);
}

int Server::implOscCallback( const char *path, const char *types, lo_arg **argv, int argc, void *data, void *userData )
{
	std::function< osc::Callback > *fn = reinterpret_cast< std::function< osc::Callback >* >( userData );

	Message m( path );
	for ( int i = 0; i < argc; i++ )
	{
		switch ( types[ i ] )
		{
			case 'i':
				m.addArg( argv[ i ]->i32 );
				break;

			case 'f':
				m.addArg( argv[ i ]->f );
				break;

			case 's':
				m.addArg( string( &argv[ i ]->s ) );
				break;

			default:
				break;
		}
	}

	return (*fn)( m ) ? 1 : 0;
}

uint32_t Server::sCallbackId = 0;

} // namespace osc

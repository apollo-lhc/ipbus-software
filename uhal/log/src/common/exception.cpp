/*
---------------------------------------------------------------------------

    This file is part of uHAL.

    uHAL is a hardware access library and programming framework
    originally developed for upgrades of the Level-1 trigger of the CMS
    experiment at CERN.

    uHAL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    uHAL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with uHAL.  If not, see <http://www.gnu.org/licenses/>.


      Andrew Rose, Imperial College, London
      email: awr01 <AT> imperial.ac.uk

      Marc Magrans de Abril, CERN
      email: marc.magrans.de.abril <AT> cern.ch

---------------------------------------------------------------------------
*/

/**
	@file
	@author Andrew W. Rose
	@date 2012
*/

#include "uhal/log/exception.hpp"


#include <cstring>
#include <iostream>
#include <sstream>
#include <stdlib.h>


namespace uhal
{
  namespace exception
  {

    exception::exception ( ) :
      std::exception (),
      mString ( ( char* ) malloc ( 65536 ) ),
      mAdditionalInfo ( ( char* ) malloc ( 65536 ) )
    {
      gettimeofday ( &mTime, NULL );
      mAdditionalInfo[0] = '\0'; //malloc is not required to initialize to null, so do it manually, just in case
    }


    exception::exception ( const exception& aExc ) :
      std::exception (),
      mTime ( aExc.mTime ),
      mString ( ( char* ) malloc ( 65536 ) ),
      mAdditionalInfo ( ( char* ) malloc ( 65536 ) )
    {
      strcpy ( mAdditionalInfo , aExc.mAdditionalInfo );
    }

    exception& exception::operator= ( const exception& aExc )
    {
      strcpy ( mAdditionalInfo , aExc.mAdditionalInfo );
      mTime = aExc.mTime;
      return *this;
    }

    exception::~exception() throw()
    {
      if ( mString )
      {
        free ( mString );
        mString = NULL;
      }

      if ( mAdditionalInfo )
      {
        free ( mAdditionalInfo );
        mAdditionalInfo = NULL;
      }
    }


    const char* exception::what() const throw()
    {
      if ( mString == NULL )
      {
        std::cout << "Could not allocate memory for exception message" << std::endl;
        return mString;
      }

      std::stringstream lStr;

      if ( strlen ( mAdditionalInfo ) )
      {
        lStr << mAdditionalInfo;
      }
      else
      {
        lStr << description() << " (no additional info)";
      }

      std::string lString ( lStr.str() );
      strncpy ( mString , lString.c_str() , 65536 );

      if ( lString.size() > 65536 )
      {
        strcpy ( mString+65530 , "..." );
      }

      return mString;
    }


    void exception::append ( const char* aCStr ) throw()
    {
      strncat ( mAdditionalInfo, aCStr , 65536-strlen ( mAdditionalInfo ) );
    }

  }
}




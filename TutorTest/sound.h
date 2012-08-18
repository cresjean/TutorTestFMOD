
#ifndef SOUND_H_
#define SOUND_H_

	#include "fmod.hpp"

	void sound_init( void );
	FMOD::Sound* sound_load( char* filename );
	void sound_play( FMOD::Sound* s, float x, float y );

#endif

#include "stdafx.h"

#include "fmod.hpp"
#include "fmod_errors.h"

#include "sound.h"

using namespace FMOD;

System* sys;

void sound_init( void )
{
	/* Listener position/orientation. */
	FMOD_VECTOR pos = { 0, 0, 0 },
				vel = { 0, 0, 0 },
				forward = { 0, 0, -1 },
				up = { 0, 1, 0 };
	
	/* Create FMOD system. */
	System_Create( &sys );

	/* Use 5.1 surround setup. */
	FMOD_RESULT r = sys->setSpeakerMode( FMOD_SPEAKERMODE_5POINT1 );
	if ( r != FMOD_OK ) {
		MessageBox( NULL, FMOD_ErrorString( r ), "Error", MB_ICONERROR | MB_OK );	
	}

	/* Disable front and sub as they're not useful for this setup. */
	sys->set3DSpeakerPosition( FMOD_SPEAKER_FRONT_CENTER,  0.0f, 0.0f, false );
	sys->set3DSpeakerPosition( FMOD_SPEAKER_LOW_FREQUENCY, 0.0f, 0.0f, false );

	/* Set corner speakers to poor approximation of keyboard corners. */
	sys->set3DSpeakerPosition( FMOD_SPEAKER_FRONT_LEFT,  -1.0f, 0.3f, true );
	sys->set3DSpeakerPosition( FMOD_SPEAKER_FRONT_RIGHT, 1.0f,  0.3f, true );
	sys->set3DSpeakerPosition( FMOD_SPEAKER_BACK_LEFT,   -1.0f, -0.3f, true );
	sys->set3DSpeakerPosition( FMOD_SPEAKER_BACK_RIGHT,  1.0f,  -0.3f, true );

	/* Init FMOD system in 3D mode. */
	sys->init( 10, FMOD_INIT_3D_RIGHTHANDED, NULL );
	sys->set3DSettings( 1.0, 1.0, 1.0 );
	sys->set3DListenerAttributes( 0, &pos, &vel, &forward, &up );
}


Sound* sound_load( char* filename )
{

	Sound* s;
	FMOD_RESULT r = sys->createSound( filename, FMOD_SOFTWARE | FMOD_3D, NULL, &s );
	if ( r != FMOD_OK ) {
		MessageBox( NULL, FMOD_ErrorString( r ), "Error", MB_ICONERROR | MB_OK );	
		return NULL;
	}

	s->set3DMinMaxDistance( 4.0, 10000.0 );
	s->setMode( FMOD_LOOP_OFF );

	return s;
}

void sound_play( Sound* s, float x, float y )
{
	FMOD_VECTOR vel = { 0, 0, 0 },
				pos = { x * 2.0 - 1.0, 0, y * 2.0 - 1.0 };

	/* Play sound at specified location. */
	Channel* channel = NULL;
	sys->playSound( FMOD_CHANNEL_FREE, s, false, &channel );
	channel->set3DAttributes( &pos, &vel );
	sys->update();
}


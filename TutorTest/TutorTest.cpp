/*
	Typing Tutor
	Test (Win32 + GDI)
*/

#include "stdafx.h"
#include "TutorTest.h"
#include "sound.h"

#define WINDOW_CLASS "TutorTest"
#define TIMER_ID     1
#define TIMER_MS	 50

/*
	Training Modes

	Mode 0: Training with auto-advance timer and all stimulus.
	Mode 1: Testing with keypress requirement and all stimulus.
*/
#define MODE_COUNT   2

typedef struct {
	int menu_id,
		use_timer, /* Advance after delay. */
		use_keys,  /* Advance after correct keypress. */
		use_sound, /* Enable sound stimulus. */
		use_text,  /* Enable text stimulus. */
		use_tens;  /* Enable electro-tactile stimulus. */
} mode_t;

static int mode = 0,
		   running = FALSE;
static mode_t modes[MODE_COUNT] = {
	{ ID_TEST_MODE0, TRUE, FALSE, TRUE, TRUE, TRUE },
	{ ID_TEST_MODE1, FALSE, TRUE, TRUE, TRUE, TRUE }
};

/*
	Training Speeds

	400 - 3000ms.
*/
#define SPEED_COUNT  8

typedef struct {
	int menu_id,
		delay_ms;
} speed_t;

static int speed = 3,
		   ticks = 0;
static speed_t speeds[SPEED_COUNT] = {
	{ ID_SPEED_400MS,  400 },
	{ ID_SPEED_600MS,  600 },
	{ ID_SPEED_800MS,  800 },
	{ ID_SPEED_1000MS, 1000 },
	{ ID_SPEED_1500MS, 1500 },
	{ ID_SPEED_2000MS, 2000 },
	{ ID_SPEED_2500MS, 2500 },
	{ ID_SPEED_3000MS, 3000 }
};

/*
	Serial Settings.
*/
#define PORT_COUNT 6

#define ID_HI  0x01
#define ID_MID 0xE2
#define ID_LO  0x40

typedef struct {
	int   menu_id;
	char* port;
} port_t;

static int port = 0;
static port_t ports[PORT_COUNT] = {
	{ ID_SERIAL_COM1, "COM1" },
	{ ID_SERIAL_COM2, "COM2" },
	{ ID_SERIAL_COM3, "COM3" },
	{ ID_SERIAL_COM4, "COM4" },
	{ ID_SERIAL_COM5, "COM5" },
	{ ID_SERIAL_COM6, "COM6" }
};

HANDLE hnd_serial = INVALID_HANDLE_VALUE;

/*
	TENS Settings.

	Levels 1 - 8 are actually mapped to
	power settings 2 - 9 for the packets
	sent to the patches.

	This is because 0 and 1 are pretty
	hard to detect on a full size patch,
	so they should be invisible on finger
	patches.
*/
#define LEVEL_COUNT 8

typedef struct {
	int menu_id,
		level;
} level_t;

static int level = 1,
		   tens_on = FALSE;
static level_t levels[LEVEL_COUNT] = {
	{ ID_TENS_LEVEL1, 2 },
	{ ID_TENS_LEVEL2, 3 },
	{ ID_TENS_LEVEL3, 4 },
	{ ID_TENS_LEVEL4, 5 },
	{ ID_TENS_LEVEL5, 6 },
	{ ID_TENS_LEVEL6, 7 },
	{ ID_TENS_LEVEL7, 8 },
	{ ID_TENS_LEVEL8, 9 }
};

/*
	Text Settings.
*/
#define TEXT_FONT    "Times New Roman"
#define TEXT_SIZE    72
#define TEXT_COLOR   RGB( 255, 255, 255 )
#define TEXT_BGCOLOR RGB( 0, 0, 0 )

static HBITMAP  text_bmp = NULL;
static HFONT    text_font = NULL;
static COLORREF text_color = TEXT_COLOR,
				text_bgcolor = TEXT_BGCOLOR,
				text_bgcustom[16] = { 0 };
static int      text_drawbg = FALSE;

/*
	Character Table.
*/
#define CHAR_COUNT			 72
#define CHAR_COUNT_NOSYMBOLS 62

#define L1 0	/* Left Pinky. */
#define L2 1	/* Left Ring. */
#define L3 2	/* Left Middle. */
#define L4 3	/* Left Index. */
#define L5 4	/* Left Thumb. */
#define R1 5	/* Right Thumb. */
#define R2 6	/* Right Index. */
#define R3 7	/* Right Middle. */
#define R4 8	/* Right Ring. */
#define R5 9	/* Right Pinky. */

typedef struct {
	int ascii;
	float hpan,
		  vpan;
	int finger;
	FMOD::Sound* s;
} char_t;

static int charc = 0;
static char_t chars[CHAR_COUNT] = {
	{ 'a',  0.17, 0.49, L1 }, { 'b',  0.46, 0.67, L4 },
	{ 'c',  0.33, 0.67, L3 }, { 'd',  0.30, 0.49, L3 },
	{ 'e',  0.28, 0.30, L3 }, { 'f',  0.37, 0.49, L4 },
	{ 'g',  0.42, 0.49, L4 }, { 'h',  0.48, 0.49, R2 },
	{ 'i',  0.59, 0.30, R3 }, { 'j',  0.55, 0.49, R2 },
	{ 'k',  0.61, 0.49, R3 }, { 'l',  0.60, 0.49, R4 },
	{ 'm',  0.57, 0.67, R2 }, { 'n',  0.52, 0.67, R2 },
	{ 'o',  0.66, 0.30, R4 }, { 'p',  0.72, 0.30, R5 },
	{ 'q',  0.15, 0.30, L1 }, { 'r',  0.35, 0.30, L4 },
	{ 's',  0.23, 0.49, L2 }, { 't',  0.41, 0.30, L4 },
	{ 'u',  0.53, 0.30, R2 }, { 'v',  0.40, 0.67, L4 },
	{ 'w',  0.23, 0.30, L2 }, { 'x',  0.27, 0.67, L2 },
	{ 'y',  0.47, 0.30, R2 }, { 'z',  0.21, 0.67, L1 },
	{ 'A',  0.17, 0.49, L1 }, { 'B',  0.46, 0.67, L4 },
	{ 'C',  0.33, 0.67, L3 }, { 'D',  0.30, 0.49, L3 },
	{ 'E',  0.28, 0.30, L3 }, { 'F',  0.37, 0.49, L4 },
	{ 'G',  0.42, 0.49, L4 }, { 'H',  0.48, 0.49, R2 },
	{ 'I',  0.59, 0.30, R3 }, { 'J',  0.55, 0.49, R2 },
	{ 'K',  0.61, 0.49, R3 }, { 'L',  0.60, 0.49, R4 },
	{ 'M',  0.57, 0.67, R2 }, { 'N',  0.52, 0.67, R2 },
	{ 'O',  0.66, 0.30, R4 }, { 'P',  0.72, 0.30, R5 },
	{ 'Q',  0.15, 0.30, L1 }, { 'R',  0.35, 0.30, L4 },
	{ 'S',  0.23, 0.49, L2 }, { 'T',  0.41, 0.30, L4 },
	{ 'U',  0.53, 0.30, R2 }, { 'V',  0.40, 0.67, L4 },
	{ 'W',  0.23, 0.30, L2 }, { 'X',  0.27, 0.67, L2 },
	{ 'Y',  0.47, 0.30, R2 }, { 'Z',  0.21, 0.67, L1 },
	{ '1',  0.13, 0.11, L1 }, { '2',  0.19, 0.11, L2 },
	{ '3',  0.25, 0.11, L3 }, { '4',  0.32, 0.11, L4 },
	{ '5',  0.37, 0.11, L4 }, { '6',  0.44, 0.11, R2 },
	{ '7',  0.50, 0.11, R2 }, { '8',  0.57, 0.11, R3 },
	{ '9',  0.63, 0.11, R4 }, { '0',  0.70, 0.11, R5 },
	{ ',',  0.00, 0.11, R3 }, { '<',  0.00, 0.00, R3 },
	{ '>',  0.00, 0.00, R4 }, { '.',  0.00, 0.00, R4 },
	{ '\"', 0.00, 0.00, R5 }, { ' ',  0.00, 0.00, R1 },
	{ ':',  0.00, 0.00, R5 }, { '?',  0.00, 0.00, R5 },
	{ ';',  0.00, 0.00, R5 }, { '\'', 0.00, 0.00, R5 }
};

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
void DoFont( HWND hwnd );
void DoBackground( HWND hwnd );
void DoDefaultFont( HWND hwnd );
void DoThink( HWND hwnd );
void DoChar( HWND hwnd, int c );
void DoCharUpdate( HWND hwnd );
void DoPaint( HWND hwnd );
void DoSerialUpdate( HWND hwnd, int port, int connect );
void DoTENSUpdate( HWND hwnd, int level, int enable );
void DoTENSShock( HWND hwnd, int finger );

#define CHECKMENUITEM( menu, id, check ) CheckMenuItem( (HMENU)menu, id, MF_BYCOMMAND | ( check ? MF_CHECKED : MF_UNCHECKED ) )

FMOD::Sound *s;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow )
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	/* Register window class. */
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor		 = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MENU );
	wc.lpszClassName = WINDOW_CLASS;
	wc.hIconSm		 = LoadIcon( NULL, IDI_APPLICATION );

	if ( !RegisterClassEx( &wc ) )
	{
		MessageBox( NULL, "Window Registration Failed.", "Error", MB_ICONERROR | MB_OK );
		return 1;
	}

	/* Create window. */
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WINDOW_CLASS,
		"TutorTest",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, hInstance, NULL );

	if ( hwnd == NULL )
	{
		MessageBox( NULL, "Window Creation Failed.", "Error", MB_ICONERROR | MB_OK );
		return 1;
	}

	/* Init DirectSound. */
    /*HRESULT hr = CoInitializeEx( NULL, 0 );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, "Failed to init DirectSound.", "Error", MB_ICONERROR | MB_OK );
		return 1;
    }
	hr = DirectSoundCreate8( NULL, &dsound, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, "Failed to init DirectSound.", "Error", MB_ICONERROR | MB_OK );
		return 1;
    }

	hr = dsound->SetCooperativeLevel( hwnd, DSSCL_PRIORITY );
	if ( FAILED( hr ) )
	{
		MessageBox( NULL, "Failed to configure DirectSound.", "Error", MB_ICONERROR | MB_OK );
		return 1;
	}*/

	sound_init();
	s = sound_load( "a.wav" );

	int i;
	for ( i = 0; i < CHAR_COUNT_NOSYMBOLS; i++ ) {
		char file[6] = { chars[i].ascii, '.', 'w', 'a', 'v', 0 };
		if ( !( chars[i].s = sound_load( file )) ) {
			MessageBox( NULL, file, "Failed to load sound", MB_ICONERROR | MB_OK );
			exit( 1 );
		}
	}

	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );

	srand( (unsigned int)time( NULL ) );

	/* Install 20fps think timer. */
	SetTimer( hwnd, TIMER_ID, TIMER_MS, NULL );

	/* Message Loop. */
	while ( GetMessage( &Msg, NULL, 0, 0 ) > 0 )
	{
		TranslateMessage( &Msg );
		DispatchMessage( &Msg );
	}

	//CoUninitialize();

	return Msg.wParam;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC hdc;
	RECT rc;
	int i;

	switch ( msg )
	{
		/* Init text and create bitmap for painting. */
		case WM_CREATE:
			DoDefaultFont( hwnd );
			GetClientRect( hwnd, &rc );
			hdc = GetDC( hwnd );
			text_bmp = CreateCompatibleBitmap( hdc, rc.right - rc.left, rc.bottom - rc.top );
			DeleteDC( hdc );
			break;

		/* Realloc bitmap on size change. */
		case WM_SIZE: 
			DeleteObject( text_bmp );
			GetClientRect( hwnd, &rc );
			hdc = GetDC( hwnd );
			text_bmp = CreateCompatibleBitmap( hdc, rc.right - rc.left, rc.bottom - rc.top );
			DeleteDC( hdc );
			break;

		/* Destroy window on close. */
		case WM_CLOSE:
			DestroyWindow( hwnd );
			break;

		/* Cleanup on exit. */
		case WM_DESTROY:
			DeleteObject( text_font );
			DeleteObject( text_bmp );
			PostQuitMessage( 0 );
			break;

		/* Menu item selected. */
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				/* Exit program. */
				case ID_TEST_EXIT:
					PostMessage( hwnd, WM_CLOSE, 0, 0 );
					break;

				/* Exit program. */
				case ID_TEST_START:
					running = TRUE;
					DoCharUpdate( hwnd );
					break;

				case ID_TEST_STOP:
					running = FALSE;
					InvalidateRect( hwnd, NULL, TRUE );
					UpdateWindow( hwnd );
					break;

				/* Change text font. */
				case ID_TEXT_FONT:
					DoFont( hwnd );
					InvalidateRect( hwnd, NULL, TRUE );
					UpdateWindow( hwnd );
					break;

				/* Change text background. */
				case ID_TEXT_BACKGROUND:
					DoBackground( hwnd );
					InvalidateRect( hwnd, NULL, TRUE );
					UpdateWindow( hwnd);
					break;

				/* Reset to default text settings. */
				case ID_TEXT_DEFAULT:
					DoDefaultFont( hwnd );
					InvalidateRect( hwnd, NULL, TRUE );
					UpdateWindow( hwnd );
					break;

				/* Close serial port if open. */
				case ID_SERIAL_DISCONNECT:
					DoSerialUpdate( hwnd, port, FALSE );
					break;

				/* Disable TENS unit. */
				case ID_TENS_DISABLE:
					DoTENSUpdate( hwnd, level, FALSE );
					break;

				/* Check to see if any of the variable
				   options have been modified. */
				default:
					/* Mode selection. */
					for ( i = 0; i < MODE_COUNT; i++ )
						if ( modes[i].menu_id == LOWORD( wParam ) )
							mode = i;
					/* Speed selection. */
					for ( i = 0; i < SPEED_COUNT; i++ )
						if ( speeds[i].menu_id == LOWORD( wParam ) )
							speed = i;
					/* TENS Level selection. */
					for ( i = 0; i < LEVEL_COUNT; i++ )
						if ( levels[i].menu_id == LOWORD( wParam ) )
							DoTENSUpdate( hwnd, i, TRUE );
					/* Serial port selection. */
					for ( i = 0; i < PORT_COUNT; i++ )
						if ( ports[i].menu_id == LOWORD( wParam ) )
							DoSerialUpdate( hwnd, i, TRUE );
					break;
			}
			break;

		/* Handle checking of menu items. */
		case WM_INITMENUPOPUP:
			for ( i = 0; i < MODE_COUNT; i++ )
				CHECKMENUITEM( wParam, modes[i].menu_id, mode == i );
			for ( i = 0; i < SPEED_COUNT; i++ )
				CHECKMENUITEM( wParam, speeds[i].menu_id, speed == i );
			for ( i = 0; i < LEVEL_COUNT; i++ )
				CHECKMENUITEM( wParam, levels[i].menu_id, level == i && tens_on );
			for ( i = 0; i < PORT_COUNT; i++ )
				CHECKMENUITEM( wParam, ports[i].menu_id, port == i && hnd_serial != INVALID_HANDLE_VALUE );
			break;

		/* Timer event. */
		case WM_TIMER: 
			/* Is it our think timer? */
			if ( wParam == TIMER_ID ) 
				DoThink( hwnd );
			break;

		/* ASCII char keypress. */
		case WM_CHAR:
			DoChar( hwnd, wParam );
			break;

		/* Block background redraws. */
		case WM_ERASEBKGND:
			return TRUE;

		/* Repaint client area. */
		case WM_PAINT:
			DoPaint( hwnd );
			break;

		/* The rest... */
		default:
			return DefWindowProc( hwnd, msg, wParam, lParam );
	}

	return FALSE;
}

void DoFont( HWND hwnd )
{
	/* Set text font and color via standard font dialog. */
	CHOOSEFONT cf = { sizeof(CHOOSEFONT) };
	LOGFONT lf;

	GetObject( text_font, sizeof(LOGFONT), &lf );

	cf.Flags = CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = text_color;

	if ( ChooseFont( &cf ) ) {
		HFONT hf = CreateFontIndirect( &lf );
		if ( hf ) {
			text_font = hf;
		} else {
			MessageBox( hwnd, "Font creation failed!", "Error", MB_OK | MB_ICONEXCLAMATION );
			PostMessage( hwnd, WM_CLOSE, 0, 0 );
		}
		text_color = cf.rgbColors;
	}
}

void DoBackground( HWND hwnd )
{
	/* Set background colour via standard colour dialog. */
	CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };

	cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
	cc.hwndOwner = hwnd;
	cc.rgbResult = text_bgcolor;
	cc.lpCustColors = text_bgcustom;

	if ( ChooseColor( &cc ) )
		text_bgcolor = cc.rgbResult;
}

void DoDefaultFont( HWND hwnd )
{
	/* Revert text to default settings. */
	HFONT hf;
	HDC hdc;
	int height;
	
	hdc = GetDC( NULL );
	height = -MulDiv( TEXT_SIZE, GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
	ReleaseDC( NULL, hdc );

	hf = CreateFont( height, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT_FONT );
	if ( hf ) {
		DeleteObject( text_font );
		text_font = hf;
		text_color = TEXT_COLOR;
		text_bgcolor = TEXT_BGCOLOR;
	} else {
		MessageBox( hwnd, "Font creation failed!", "Error", MB_OK | MB_ICONEXCLAMATION );
		PostMessage( hwnd, WM_CLOSE, 0, 0 );
	}					
}

void DoThink( HWND hwnd )
{
	if ( !running || !modes[mode].use_timer )
		return;

	ticks++;					
	if ( ticks * TIMER_MS < speeds[speed].delay_ms )
		return;

	DoCharUpdate( hwnd );
}

void DoChar( HWND hwnd, int c )
{
	if ( !running || !modes[mode].use_keys )
		return;

	if ( c != chars[charc].ascii )
		return;

	DoCharUpdate( hwnd );
}

void DoCharUpdate( HWND hwnd )
{
	charc = rand() % CHAR_COUNT_NOSYMBOLS;
	ticks = 0;
	
	if ( running && modes[mode].use_tens )
		DoTENSShock( hwnd, chars[charc].finger );
	
	/*char name[6] = " .wav";
	if ( running && modes[mode].use_sound && charc < CHAR_COUNT_NOSYMBOLS ) {
		name[0] = tolower( chars[charc].ascii );
		PlaySound( name, NULL, SND_FILENAME | SND_ASYNC );
	}*/

	if ( running && modes[mode].use_sound && charc < CHAR_COUNT_NOSYMBOLS ) {
		sound_play( chars[charc].s, chars[charc].hpan / 0.72, chars[charc].vpan / 0.67 );
	}		

	InvalidateRect( hwnd, NULL, TRUE );
	UpdateWindow( hwnd );
}

void DoPaint( HWND hwnd )
{
	RECT rc;
	PAINTSTRUCT ps;	
	HDC hdc, bhdc;
	HBRUSH bg_brush;

	char str[2] = { chars[charc].ascii, 0 };

	/* Setup. */
	GetClientRect( hwnd, &rc );
	hdc = BeginPaint( hwnd, &ps );
	bhdc = CreateCompatibleDC( hdc );
	SelectObject( bhdc, text_bmp );

	/* Paint Background. */
	bg_brush = CreateSolidBrush( text_bgcolor );
	FillRect( bhdc, &rc, bg_brush );
	DeleteObject( bg_brush );

	/* Paint Text if necessary. */
	if ( running && modes[mode].use_text ) {
		SelectObject( bhdc, text_font );
		SetTextColor( bhdc, text_color );
		SetBkMode( bhdc, TRANSPARENT );
		DrawText( bhdc, str, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
	}

	/* Copy to client area. ( Painting was done off-screen to prevent flicker. ) */
    BitBlt( hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, bhdc, 0, 0, SRCCOPY );

	/* Cleanup. */
	DeleteDC( bhdc );
	EndPaint( hwnd, &ps );
}

void DoSerialUpdate( HWND hwnd, int nport, int connect )
{
	DCB conf = { 0 };
	conf.DCBlength = sizeof( conf );

	if ( hnd_serial != INVALID_HANDLE_VALUE ) {
		DoTENSUpdate( hwnd, 0, FALSE );
		FlushFileBuffers( hnd_serial );
		CloseHandle( hnd_serial );
		hnd_serial = INVALID_HANDLE_VALUE;
		if ( !connect )
			MessageBox( hwnd, "Connect closed.", "Serial", MB_OK | MB_ICONINFORMATION );
	} else {
		level = 0;
		tens_on = FALSE;
	}

	if ( !connect )
		return;

	port = nport;

	hnd_serial = CreateFile( ports[port].port, GENERIC_READ | GENERIC_WRITE, 0, 0,
							 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( hnd_serial == INVALID_HANDLE_VALUE ) {
		MessageBox( hwnd, "Failed to open serial port.", "Error", MB_OK | MB_ICONERROR );
		return;
	}

	if ( !GetCommState( hnd_serial, &conf ) ) {
		MessageBox( hwnd, "Failed to configure serial port.", "Error" , MB_OK | MB_ICONERROR );
		CloseHandle( hnd_serial );
		hnd_serial = INVALID_HANDLE_VALUE;
		return;
	}

	conf.BaudRate = CBR_19200;
	conf.ByteSize = 8;
	conf.Parity = NOPARITY;
	conf.StopBits = ONESTOPBIT;

	if ( !SetCommState( hnd_serial, &conf ) ) {
		MessageBox( hwnd, "Failed to configure serial port.", "Error" , MB_OK | MB_ICONERROR );
		CloseHandle( hnd_serial );
		hnd_serial = INVALID_HANDLE_VALUE;
		return;
	}

	MessageBox( hwnd, "Connected at 19200 baud, 8/N/1.", "Serial", MB_OK | MB_ICONINFORMATION );
}

void DoTENSUpdate( HWND hwnd, int nlevel, int enable )
{
	unsigned char packet[7] = { 't', ID_HI, ID_MID, ID_LO, 'p', enable, levels[level].level };
	DWORD nbytes;

	if ( hnd_serial == INVALID_HANDLE_VALUE ) {
		MessageBox( hwnd, "Not connected to TENS Unit.", "Error", MB_OK | MB_ICONERROR );
		return;
	}

	if ( WriteFile( hnd_serial, packet, 7, &nbytes, NULL ) ) {
		MessageBox( hwnd, "Error writing to serial port.", "Error", MB_OK | MB_ICONERROR );
		CloseHandle( hnd_serial );
		hnd_serial = INVALID_HANDLE_VALUE;
		level = 0;
		tens_on = FALSE;
		return;
	}

	level = nlevel;
	tens_on = enable;
}

void DoTENSShock( HWND hwnd, int finger )
{
	unsigned char packet[6] = { 't', ID_HI, ID_MID, ID_LO, 'f', finger };
	DWORD nbytes;

	if ( hnd_serial == INVALID_HANDLE_VALUE )
		return;

	if ( WriteFile( hnd_serial, packet, 6, &nbytes, NULL ) ) {
		MessageBox( hwnd, "Error writing to serial port.", "Error", MB_OK | MB_ICONERROR );
		CloseHandle( hnd_serial );
		hnd_serial = INVALID_HANDLE_VALUE;
		level = 0;
		tens_on = FALSE;
		return;
	}
}

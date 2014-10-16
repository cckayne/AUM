{ visual: verify the likely goodness of selected PRNGs from their visual patterns. }
program visual;
{$mode delphi}

uses crt, sysutils, rng, graph256_sdl, sdl, sdlutils;

var 
	caption: string  = 'PRNG Visual Test';
	testnum: integer= 1;
	fullscr: boolean = false;
	xscreen: integer = 1024;
	yscreen: integer = 768;
	numpoints: Cardinal = 118200;
	pause: integer = 7000;
	
	video_flags: Uint32;
	screen_: PSDL_Surface;
	//Pal: T256Palette;
	
	xx,yy: integer;
	colour,bgcolour: Cardinal;
	sdlresult: integer;
	
	seed: string = 'Something very silly';
	rng: TRNG = OM;

// Set one pixel of a given colour
procedure putpixel(x,y: integer; c: cardinal);
  begin
	try
		if SDL_MUSTLOCK(screen_) then
			SDL_LockSurface(screen_);
		
		SDL_PutPixel(screen_,x,y,c);
			
		if SDL_MUSTLOCK(screen_) then
			SDL_UnlockSurface(screen_);
		// Update only the part of the display we've changed
		SDL_UpdateRect(screen_, xx, yy, 1, 1);
	except
	end;
end;
	
// Clear the entire display area with a colour
procedure cleardisplay(bg: Cardinal);
	var	rect_  : TSDL_Rect;
	begin
		with rect_ do begin
			x := 1; y :=1;
			w := xscreen;
			h := yscreen;
		end;
		try
			if SDL_MUSTLOCK(screen_) then
				SDL_LockSurface(screen_);
			sdlresult:=SDL_FillRect(screen_,@rect_,bgcolour);
			if SDL_MUSTLOCK(screen_) then
				SDL_UnlockSurface(screen_);
			// Update the entire display area
			SDL_UpdateRect(screen_, 0, 0, 0, 0);
		except
			if (sdlresult<>0) then
			 Writeln('SDL Error >> SDL_FillRect <<');
		end;
	end;


function Normal(r:TRNG; max: integer): integer;
	var n,tot,lim: integer;
	begin
		lim := 2;
		tot := 0;
		for n := 1 to lim do
			tot += rRand(r,max);
		result := tot div lim;
	end;


// Zero-one test
procedure test1(rng: TRNG);
	var one,zero: integer;
		r: Cardinal;
	begin
		Writeln(rngs[rng],' zero-one test.');
		one:=0; zero:=0;
		for yy := 0 to yscreen-1 do
			for xx := 0 to xscreen-1 do begin
				r := rRandom(rng) and 1;
				if r=1 then begin 
					inc(one);
					putpixel(xx,yy,colour);
				end else inc(zero);
			end;
		Writeln('Zero = ',zero,' One = ',one);	
	end;
	

// Random scatter test	
procedure test2(rng: TRNG);
	var n: cardinal;
	begin
		Writeln(rngs[rng],' random scatter test.');
		for n := 1 to numpoints do begin
			xx := rRandom(rng) mod xscreen;
			yy := rRandom(rng) mod yscreen;
			putpixel(xx, yy, colour);
		end;
	end;
	
	
PROCEDURE Usage;
	BEGIN
		Writeln;
		Writeln('Usage:   >visual <rng> <seed> <test #> <fullscreen?> <x-rez> <y-rez>');
		Writeln('Example: >visual 4 "Babbage" 1 0 1024 768');
		Writeln('Available RNGs: [0..14] 2=ISAAC 3=BB256 4=BB512 10=AUM...');
		Writeln('All command line parameters are optional.');
		Writeln;
	END;

begin
	// pull RNG & screen parameters from the command line
	if ParamCount>=1 then if ParamStr(1)[1] in ['a'..'z','A'..'Z','-'] then begin Usage; Halt; end;
	if ParamCount>=1 then rng := TRNG(StrToInt(ParamStr(1)));
	if ParamCount>=2 then seed := ParamStr(2);
	if ParamCount>=3 then testnum := StrToInt(ParamStr(3));
	if ParamCount>=4 then if StrToInt(ParamStr(4))=1 then fullscr:=true else fullscr:=false;
	if ParamCount>=5 then xscreen := StrToInt(ParamStr(5)); 
	if ParamCount>=6 then yscreen := StrToInt(ParamStr(6)); 
	
	// Initialize the SDL library
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) then begin
		Writeln('Could not initialize SDL');
		SDL_Quit;
		exit;
	end;
 
	// Video & screen mode
	if fullscr then
		 video_flags := SDL_SWSURFACE or SDL_FULLSCREEN
	else video_flags := SDL_SWSURFACE;
	
	screen_ := SDL_SetVideoMode(xscreen, yscreen, 32, video_flags);
	if ( screen_  = nil ) then begin
		writeln('Could not initialize video mode');
		SDL_Quit;
		exit;
	end;
	// Set the SDL window caption and icon name
	caption := rngs[rng]+' test #'+IntToStr(testnum)+' in '+IntToStr(xscreen)+'x'+IntToStr(yscreen);
	SDL_WM_SetCaption(pchar(caption),'ses.ico');
  
	// Map the colour to this display (R=0xff, G=0xFF, B=0x00)
	//	Note:  If the display is palettized, the palette must be set first. }
	bgcolour:= SDL_MapRGB(screen_^.format, 0, 13, 7);
	colour 	:= SDL_MapRGB(screen_^.format, 0, 128, 64);

	// Clear the screen
	cleardisplay(bgcolour);
	
	// Seed all PRNGs and conduct the selected test
	rSeedAll(seed);
	if testnum=1 then test1(rng) else test2(rng);

	// allow the image to sink in
	delay(pause);
	// Clean up and free resources
	SDL_Quit;
end.

{ Small, fast PRNGs by Bob Jenkins }
unit rJenkins;

interface

// return a 32 bit unsigned uniformly distributed random number
function flea: Cardinal;
// initialize Flea PRNG using a pair of seeds
procedure initFlea(a,b: Cardinal);
// return a CS 32-bit random number	
function csfRandom: Cardinal;
// initialize CS Flea PRNG with a string
procedure csfInit(seed: string);

implementation


// Flea globals
var v, w, x, y, z: Cardinal;
// csFlea globals
	state: array[0..255] of Cardinal;
	rndrs: array[0..255] of Cardinal;
	
	b, c, d, e, rcnt: Cardinal;


// return a 32 bit unsigned uniformly distributed random number
function flea: Cardinal;
	begin
		v := w;
		w := x;
		x := ((y shl 19) + (y shr 13)) + z;
		y := z xor w;
		z := v + x;
		flea := y;
	end;


// initialize the Flea random number generator using a pair of seeds
procedure initFlea(a,b: Cardinal);
	var i: cardinal;
	begin
		v := 314159;
		w := 265358;
		x := 932384;
		y := 643383;
		z := 279502;
		z += a;
		x += b;
    	for i:=0 to (((a+b) mod 1000)+1000)-1 do flea;
	end;

	
// csFlea is called, like ISAAC, every 255 rounds of rcnt;	
procedure csFlea;
	var i: Cardinal;
    begin    
		for i := 0 to 255 do begin
			e := state[d mod 256];
			state[d mod 256] := b;
			b := (c<<9) + (c>>23) + d;
			c := d xor state[i];
			d := e + b;
			rndrs[i] := c;
		end;
    end;
	
	
// return a CS 32-bit random number	
function csfRandom: Cardinal;
	begin
		csfRandom := rndrs[rcnt];
		inc(rcnt);
		if rcnt>255 then begin
			rcnt := 0;
			csFlea;
		end;
	end;
	

procedure csfInit(seed: string);
	var i,l,w1,w2: cardinal;
	begin
		l := Length(seed);
		rcnt := 0;
		// put first 8 bytes of the seed into 2 words
		w1 := cardinal(ord(seed[1])) shl (3*8);
		w1 := w1 or (cardinal((ord(seed[2]))) shl (2*8));
		w1 := w1 or (cardinal((ord(seed[3]))) shl (1*8));
		w1 := w1 or cardinal(ord(seed[4]));
		
		w2 := cardinal(ord(seed[5])) shl (3*8);
		w2 := w2 or (cardinal((ord(seed[6]))) shl (2*8));
		w2 := w2 or (cardinal((ord(seed[7]))) shl (1*8));
		w2 := w2 or cardinal(ord(seed[8]));
		
		// initialize Flea with the two words
		initFlea(w2,w1);
		
		// fill state with Flea output
		for i := 0 to 255 do
			state[i] := Flea;
		// OR the state with the seed & xor with Flea
		for i := 0 to 255 do
			if i<l then state[i] := (state[i] or ord(seed[i+1])) xor Flea;
		// stir the state
		for i := 0 to 4096 do
			csfRandom;
	end;

initialization

finalization

end.

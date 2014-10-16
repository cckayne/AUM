// m: Check output from the AUM CSPRNG 
program au;
uses aum;
{ $define STREAM}
// mixing rounds: 7 is recommended minimum
const ROUNDS = 7;
var n: Cardinal;
	seed: string = 'Bacon';
begin
	// pull a seed from the command line
	if ParamCount>0 then seed:=ParamStr(1);
	// print some information
	Writeln('CSPRNG        : ',auName);
	Writeln('Internal state: ',auStateBits,' bits');
	Writeln('Minimum cycle : 2**',auCycle);
	Writeln('Maximum cycle : 2**',auStateBits);
	Writeln('Average cycle : 2**',auStateBits-2);
	// seed MOTE and mix
	auSeedW(seed,8*ROUNDS);
	{$ifdef STREAM}
	// show some MOD 26 stream output
	for n:=1 to 672 do
		Write(chr(auRandom mod 26 + ord('A')));
	Writeln;
	{$endif}
end.

{$MODE DELPHI}

{ Emulation of 256 colour RGB palettes in Win95/Delphi }
{******************************************************}

Unit graph256_sdl;

interface

Const MaxCol = 255;
      MinCol = 1;   //was 0

type TShortStr = string[32];

type T256PalReg   = object

     value : integer;

     procedure SetValue(val: integer);
     function  GetValue: integer;

     procedure Increment;
     procedure Decrement;
     procedure Add(n: integer);
     procedure Subtract(n: integer);
     procedure Divide(n: integer);
     procedure Multiply(n: integer);

end; { T256PalReg }




type P256PalEntry = ^T256PalEntry;
     T256PalEntry = object

     red    : T256PalReg;
     green  : T256PalReg;
     blue   : T256PalReg;

     mapping : longint;

    { Record colour's programmed function }
     AssignmentStr : ^TShortStr;
     Assignment    : byte;


     Procedure Init;
     Procedure Reset;
     Procedure Destroy;

    { Map to Windows palette and store value }
     Procedure Map;
    { Convert mapping back to RGB }
     Procedure Unmap(var r,g,b: byte);

    { Add/Subtract n uniformly from each register }
     Procedure Up(n: integer);
     Procedure Down(n: integer);

     Procedure SetRGBColour(r,g,b: byte);
     Procedure SetWinColour(value : longint);

end; { T256PalEntry }




type T256PalArray = array [0..MaxCol] of T256PalEntry;
     P256PalArray = ^T256PalArray;


type P256Palette = ^T256Palette;
     T256Palette = object

     Entry   : P256PalArray;

     InitOk  : boolean;
     Filename: string;
     Title   : string;

     Constructor Init(Tit,FName: string);
     Destructor  Done;

     Procedure Load;
     Procedure Save;

     Procedure Ramp(n,rfirst,rlast: integer);

end; { T256Palette }





implementation

uses SysUtils;



{ T256PalEntry  M E T H O D S }

Procedure T256PalEntry.Init;
begin

     red.SetValue(0);
     green.SetValue(0);
     blue.SetValue(0);

     mapping := 0;

     new(AssignmentStr);
     AssignmentStr^ := '';
     Assignment    := 0;

end; { T256PalEntry.Init }




procedure T256PalEntry.Destroy;
begin

   dispose(AssignmentStr);

end; { T256PalEntry.Destroy }





Procedure T256PalEntry.Reset;
begin
     red.SetValue(0);
     green.SetValue(0);
     blue.SetValue(0);

     mapping := 0;

     AssignmentStr^ := '';
     Assignment    := 0;

end; { T256PalEntry.Reset }



{ Map to Windows palette and store value }
Procedure T256PalEntry.Map;
begin
     mapping := 0 or (Blue.value shl 16) or (green.value shl 8)
                  or (Red.value);

end; { T256PalEntry.Map }




{ Convert mapping back to RGB }
Procedure T256PalEntry.Unmap(var r,g,b: byte);
begin



end; { T256PalEntry.Unmap }




{ Add/Subtract n uniformly from each register }

Procedure T256PalEntry.Up;
begin
     Red.Add(n);
     Blue.Add(n);
     Green.Add(n);
    { Map entry to Windows colours }
     Map;

end; { T256PalEntry.Up }




Procedure T256PalEntry.Down;
begin
     Red.Subtract(n);
     Green.Subtract(n);
     Blue.Subtract(n);
    { Map entry to Windows colours }
     Map;

end; { T256PalEntry.Down }




Procedure T256PalEntry.SetRGBColour{(r,g,b: byte)};
begin



end; { T256PalEntry.SetRGBColour }





Procedure T256PalEntry.SetWinColour{(value: longint)};
begin



end; { T256PalEntry.SetWinColour }






{ T256PalReg  M E T H O D S  }

procedure T256PalReg.SetValue{(val: byte)};
begin

   If val>255 then value := 255 else
    value := val;

end; { T256PalReg.SetValue }



function  T256PalReg.GetValue;
begin

    GetValue := Value;

end; { T256PalReg.GetValue }



procedure T256PalReg.Increment;
begin
     inc(value);
     if value >= 255 then
        value := 0;
end; { T256PalReg.Increment }




procedure T256PalReg.Decrement;
begin
     dec(value);
     if value<=0 then
        value:= 255;

end; { T256PalReg.Decrement }




procedure T256PalReg.Subtract;
begin
     value := value - n;
     if value < MinCol then
        value := 0;

end; { T256PalReg.Subtract }




procedure T256PalReg.Add;
begin
     value := value + n;
     if value > MaxCol then
        value := MaxCol
     else if
        value < MinCol then
        value:= MinCol;

end; { T256PalReg.Add }




procedure T256PalReg.Divide;
begin
     value := value div n;
     if value <MinCol then
        value:=MinCol;

end; { T256PalReg.Divide }




procedure T256PalReg.Multiply;
begin
     value := value*n;
     if value >MaxCol then
        value:=MaxCol;

end; { T256PalReg.Multiply }









{ T256Palette  M E T H O D S }

Constructor T256Palette.Init{(Tit: string)};
var count: integer;

begin
     Filename := FName;
     Title    := Tit;

     new(Entry);

     for count := MinCol to MaxCol do
       Entry[count].init;

     InitOk := FileExists(FName);

end; { T256Palette.Init }




Destructor  T256Palette.Done;
var count: integer;
begin

     for count := MinCol to MaxCol do
       Entry[count].Destroy;

    {dispose(Entry);}

end; { T256Palette.Done }





Procedure T256Palette.Load;


procedure LoadMapPalette;     {Fractint *.MAP files}
{-----------------------}     {with comments excised}
var RedLevel,GreenLevel,BlueLevel:    LONGINT;
    count                        :    byte;
    Mapfile                      :    text;
    col,row                      :    byte;

begin
     AssignFile(Mapfile,Filename);

     Reset(Mapfile);


     For count := MinCol to MaxCol do begin

        { for palette grid }
         row := MaxCol div count;
         col := MaxCol mod count;

             Read(Mapfile,RedLevel,GreenLevel,BlueLevel) ;

             Entry[count].red.SetValue(RedLevel);
             Entry[count].green.SetValue(GreenLevel);
             Entry[count].blue.SetValue(BlueLevel);

           { construct 4-byte Windows colour value }
            With Entry[count] do begin
             mapping := 0 or (BlueLevel shl 16) or (greenLevel shl 8)
                     or (RedLevel);

            end; { With }

         end;

     CloseFile(MapFile);


end; { Local }


begin

     LoadMapPalette;

end; { T256Palette.Load }




Procedure T256Palette.Save;
begin



end; { T256Palette.Save }



Procedure T256Palette.Ramp;
var c: integer;
begin
     for c := rfirst to rlast do
         with Entry[c] do
           Up(n);

end; { T256Palette.Ramp() }





end.


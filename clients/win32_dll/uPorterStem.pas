unit uPorterStem;
{ ==================================================================
  You may distribute this module but please leave the whole of this
  initial comment block intact.
  ==================================================================
  Delphi unit uPorterStem

  by Jo Rabin, jo*rabin&btclick*com

  version 1.01, 13 April 2004

  Copyright (c) 2004 Jo Rabin
  ==================================================================
  References:

  = for info about Porter stemming see http:snowball.tartarus.org

  = for specific info on the 'original' algoritm see
    http://www.tartarus.org/~martin/PorterStemmer/

    Aside from the algoritm itself, this work is derivative
    of the programs you will find there - though it differs in a
    number of respects.

    The module  implements 'enhanced' algorithm i.e. the original article
    plus three changes in the later 'C' module by M Porter
  ==================================================================
  Notes

  = tested Delphi 5 only

  = should be thread safe

  = tested on the supplied test set with 0 errors

  = runs slightly faster than the non threadsafe 'C' module when compiled
    with endsin and H-

  = endsin is a small assembler routine to see if the passed string ends in
    a test string.

    Remove the define 'endsin' if you don't like assembler and you want
    to get a 'pure pascal' implementation. The difference in speed is
    around 15% as measured against the test set

  = The code runs a bit faster if we use shortstrings for internal string
    consts. The switch H- is a little way below the start of the module,
    so that the string declarations in the overloaded interface routine
    compile in whatever string mode is the default at the top of the module

  ================================================================== }

interface

Function PorterStem(const s: string): string; overload;
Function PorterStem(p: pchar; first, last: integer): integer; cdecl; overload;

implementation
  uses sysutils;

function PorterStem(const s: string): string; overload;
var
  l: integer;
begin
  l := PorterStem(@s[1],0,length(s)-1);
  result := s;
  Setlength(result,l+1);
end;

// see above before changing these settings
{#define endsin}
{#H-}


// ================================================================
// PorterStem
// a lot of local procedures - finally followed by the main body
// ================================================================
const vowels = ['a','e','i','o','u'];


function PorterStem(p: pchar; first,last: integer):integer; cdecl; overload;

// isVowel returns true if p[i] is a vowel or a y not preceded by a vowel
function isVowel(index: integer): boolean;
begin
  if (p[index] in vowels) or
     (p[index] = 'y') and (index > first) and
         not (p[index-1] in vowels) then
    result := true
  else
    result := false;
end;

// if there is a vowel in the string <= index

function hasVowel(index: integer): boolean;
var i: integer;
begin
   result := true;
   for i := index downto first do
      if isVowel(i) then exit;
   result := false;
end;

// counts the number of VC patterns in string where string is [C]{VC}[V]
// and C is any number of contiguous consonants and V is analogous
// don't count beyond m=2 as we don't care
function getM(index: integer): integer;
var
  oldmode: boolean;
  newmode: boolean;
  i: integer;
begin
  oldmode := true;
  result := 0;
  for i := index downto first do
  begin
     newmode := isVowel(i);
     // if new is vowel old was not  then inc m
     // strangely the test (not oldmode and newmode) is slower
     if (newmode <> oldmode) and newmode then
     begin
       inc(result);
       if result > 1 then exit;
     end;
     oldmode := newmode;
  end;
end;

// test for cvc at end of string where second c is not WXY
// c is a single consonant v is a single vowel

function oRule(index: integer): boolean;
begin
  if (index < first + 2) or
     isVowel(index) or
     not isVowel(index-1) or
     isVowel(index-2) or
     (p[index] in ['w','x','y'] ) then
        result := false
  else
     result := true;
end;

{$ifdef endsin}

// routine endsin has two entry points - one is when it is called from main
// body of outer procedure, the other is for when it is called from
// another local procedure. The reason for this is that the local symbols
// l, first and last are accessed at offsets from the outer procedure's
// bp. if called from an internal procedure the outer proc's bp needs to
// be restored before entry. The outer proc's bp is passed as a hidden
// parameter to local procedures and is [ebp+8]

// Incidentally, this is why is it quite a lot quicker to put all the
// step1 step2 etc. procedures in-line in the outer procedure rather than
// modularizing them as in the original - viz every access the compiler
// encodes for (say) p involves indirecting the access through ebp+8
// whereas in the outer procedure it is a straight offset from ebp.
//
// so now you know.

// at entry eax is comp
// ebp is assumed to allow correct access to the local variables
function endsinCommon(const comp: string):boolean;
asm
  push ebx
  mov  ecx,last        // set ecx to 'last'

  mov  ebx,ecx
  sub  ebx,first // subtract 'first'
  inc  ebx             // ebx is length remaining

  // adjust string pointer to end i.e. add L, subtract length(comp)
  mov  edx,p           // set edx to 'p'
  add  edx,ecx         // add last

  // set ecx to length of the comparison string
  {$ifopt H+}
  mov  ecx,[eax-4]
  cmp  ebx,ecx
  {$else}
  movzx  ecx,byte [eax]
  cmp  bl, cl
  {$endif}
  // if not enough characters, exit
  jle   @exit

  sub  edx,ecx            // sub length comparison string

  // edx now points to char before comparison

  {$ifopt H+}
  // decrement because L is a 1 based index
  dec  eax
  {$else}
  {eax already points to 1 before first char}
  {$endif}

  // loop until no match or all characters done
  // tried REP CMPSB but was slower (much)
@test:
  mov  bl,byte [eax+ecx]
  cmp  byte [edx+ecx],bl
  loopz @test
  // test for no match
  jnz  @exit
  // result is TRUE
  mov  eax,1
  pop  ebx
  pop  ebp
  ret
@exit:
  xor eax,eax
  pop ebx
  pop ebp
  ret
end;

// call this one from outer proc
function endsinl(const comp: string):boolean;
asm
  push ebp       // just so that the exit from the main routine is consistent
  jmp  endsincommon
end;

// call this one from inner proc
function endsin(const comp: string): boolean;
asm
  // set edx to old base pointer - of enclosing routine
  // this is pushed as a silent parameter at ebp+8
  push  ebp
  mov   ebp,[ebp+8]
  jmp   endsincommon
end;
{$endif}


// replaces ss at the end of p with t if getM > 0
// returns true if ss matches end of p (i.e. whether or not the
// replacement was carried out)

function CReplace(const ss, t: string): boolean;
var
  lalss: integer;
begin
  {$ifdef endsin}
  result := endsin(ss);
  {$else}
  result := comparemem(@ss[1],@p[last-length(ss)+1],length(ss));
  {$endif}
  if result then
  begin
     lalss := last-length(ss);
     if getM(lalss) > 0 then
     begin
       move(t[1],p[lalss +1],length(t));
       last := lalss + length(t);
     end;
  end;
end;

// trims ss from the end of p if getM > 0
// returns true if match irrespective the value of m

function CTrim0(const ss: string):boolean;  overload;
begin
  {$ifdef endsin}
  result := endsin(ss);
  {$else}
  result := comparemem(@ss[1],@p[last-length(ss)+1],length(ss));
  {$endif}
  if result then
  begin
    if  getM(last-length(ss)) > 0 then
      dec(last,length(ss));
  end;
end;

// trims ss from the end of p if getM > 1
// return true if ss matched

function CTrim1(const ss: string):boolean;  overload;
begin
  {$ifdef endsin}
  result := endsin(ss);
  {$else}
  result := comparemem(@ss[1],@p[last-length(ss)+1],length(ss));
  {$endif}
  if result then
  begin
    if  getM(last-length(ss)) > 1 then
      dec(last,length(ss));
  end;
end;

// trims ss from p irrespective the value of m
// returns true if a match

function CTrim(const ss: string):boolean;  overload;
begin
  {$ifdef endsin}
  result := endsin(ss);
  {$else}
  result := comparemem(@ss[1],@p[last-length(ss)+1],length(ss));
  {$endif}
  if result then
     dec(last,length(ss));
end;

// consts for the following comparemems  (Steps 1a and 1b2)

const
   ies:  string = 'ies';
   sses: string = 'sses';
   at:   string = 'at';
   bl:   string = 'bl';
   iz:   string = 'iz';

procedure Step1b2;
begin
  if last < first+2 then exit;

  // does string end in at, bl or iz?
  {$ifdef endsin}
  if endsin(at) or
     endsin(bl) or
     endsin(iz) then
  {$else}
  if comparemem(@p[Last-1],@at[1],2) or
     comparemem(@p[Last-1],@bl[1],2) or
     comparemem(@p[Last-1],@iz[1],2) then
  {$endif}
  begin
    inc(last);
    p[last] := 'e';
    exit;
  end;

  // end in double consonant?
  if (p[last] = p[last-1]) and not isVowel(last) then
  begin
    if not (p[last] in ['l','s','z']) then
      dec(last);
    exit;
  end;

  // m=1 and o
  if (getM(last) = 1) and oRule(last) then
  begin
    inc(last);
    p[last] := 'e';
  end;
end;


// ========= start of porterstem ====================
var
  m: integer;
begin
  if last-first < 2 then
  begin
     result := last;
     exit;
  end;

  // ============== Step 1

  // == step 1a
  if p[last] = 's' then
  begin
  {$ifdef endsin}
    if endsinl(ies) or
       endsinl(sses) then
  {$else}
    if (last > first + 2) and comparemem(@p[Last-2],@ies[1],3) or
       (last > first + 3) and comparemem(@p[Last-3],@sses[1],4) then
  {$endif}
      dec(last,2)
    else if p[last-1] <> 's' then
      dec(last);
  end;

  // == step 1b
  case p[last-1] of
  'e': if not cReplace('eed','ee') then
       if hasVowel(last-2) and cTrim('ed') then
         step1b2;
  'n': if hasVowel(last-3) and cTrim('ing') then
         step1b2;
  end;

  // == step 1c
  if (p[last] = 'y') and hasVowel(last-1) then
      p[last] :='i';

  // =========== step 2
  case p[last-1] of
  'a': if not  cReplace('ational','ate') then
               cReplace('tional','tion');
  'c': if not  cReplace('enci','ence') then
               cReplace('anci','ance');
  'e':         cReplace('izer','ize');
  'g':         cReplace('logi','log');
  'l': if not  cReplace('bli','ble') then
  //   if not  cReplace('abli','able') then
       if not  cReplace('alli','al') then
       if not  cReplace('entli','ent') then
       if not  cReplace('eli','e') then
               cReplace('ousli','ous');
  'o': if not  cReplace('ization','ize') then
       if not  cReplace('ation','ate') then
               cReplace('ator','ate');
  's': if not  cReplace('alism','al') then
       if not  cReplace('iveness','ive') then
       if not  cReplace('fulness','ful') then
               cReplace('ousness','ous');
  't': if not  cReplace('aliti','al') then
       if not  cReplace('iviti','ive') then
               cReplace('biliti','ble');
  end;

  // ============= Step3
  Case p[last] of
   'e': if not cReplace('icate','ic') then
        if not   cTrim0('ative') then
               cReplace('alize','al');
   'i':        cReplace('iciti','ic');
   'l': if not   cTrim0('ful') then
               cReplace('ical','ic');
    's':         cTrim0('ness');
  end;

  // ============= Step4
  case p[last] of
    'c':          cTrim1('ic');
    'e':
      case p[last-2] of
      'n': if not cTrim1('ance') then
                  cTrim1('ence');
      'b': if not cTrim1('able') then
                  cTrim1('ible');
      'a':        cTrim1('ate');
      'i': if not cTrim1('ive') then
                  cTrim1('ize');
      end;
    'i':          cTrim1('iti');
    'l':          cTrim1('al');
    'm':          cTrim1('ism');
    'n':  if (p[last-3] in ['s','t']) then
                  cTrim1('ion');
    'r':          cTrim1('er');
    's':          cTrim1('ous');
    't':
       case p[last-2] of
      'a':        cTrim1('ant');
      'e': if not cTrim1('ement') then
           if not cTrim1('ment') then
                  cTrim1('ent');
       end;
    'u':          cTrim1('ou');
  end;

  // ============= Step5a

  if (p[last] = 'e') then
  begin
    m := getM(last-1);
    if (m > 1) or (m = 1) and not oRule(last-1) then
       dec(last);
  end;

  // =========== Step5b
  if (p[last] = 'l') and (p[last-1] = 'l') and (getM(last) > 1) then
    dec(last);

  Result := last;
end;


end.



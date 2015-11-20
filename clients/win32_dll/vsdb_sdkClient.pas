unit vsdb_sdkClient;

interface
uses Classes,SysUtils,dialogs, Graphics, Controls,uPorterStem,md5,VSDB_SDK ;

type
TvsdbSDKClient = class(TComponent)
 private
   busy:boolean; 
   fMessage:tstringlist;
   sessionid:string;
   VSDB:pointer;fstoplist:tstringlist;

 protected
   procedure BeginVector(VectorID:String;Dimensions:integer);
   procedure BeginQuery(QueryID:String;Dimensions:integer);
   procedure DeleteVector(DocumentID:String);
   procedure AddDimension(DimensionID:String;Value:single);
   procedure SendQuery(QueryID:string;Threshold:single;resultlist:tstrings); {results come back DocumentID=Score}
   procedure RefreshCache;
   function  CreateID:string;
   function  NullID:string;

   procedure Rollback;
   procedure Commit;
 public
   test_encoded:tstringlist;
   function  CreateMessage(Command:char;ID:String;value:single):string;
   procedure SendMessage(Msg:String;expectReply:boolean;Results:TStrings);
   constructor create(Aowner:TComponent);override;
   destructor destroy;override;
   procedure  test_StartQuery(DocumentID:TGUID;Text:String);
   procedure  test_CompleteQuery;
   function InStopList(token:String):boolean;
   function RawInStopList(token:String):boolean;
   property Stoplist:tstringlist read fstoplist;      
   procedure EncodeText(text: string;Vector:tstringlist);
   procedure Mode(modecode:integer;thesaurus_threshold:single);
 published
   function Codelist(Text:string):tstringlist;
   function RawEncoded(Text:string):string;
   function RawScript(DocumentID:TGUID;Text:String):string;
   procedure ReplaceNameValues(DocumentID:TGUID;data:tstringlist);
   procedure Replace(DocumentID:TGUID;Text:String);
   procedure Query(QueryID:TGUID;Threshold:single;Text:String; Results:TStrings);
   procedure Clusters(segment,segmentsize:integer;cluster_size_min:integer;threshold:single;Results:TStrings);
   procedure Delete(DocumentID:TGUID);
   procedure LoadFromFile(filename:string);
   procedure SaveToFile(filename:string);
   procedure Dump(s:tstrings);
 end;

   function  GuidToID(ID:TGUID):string;
   function  IDToGUID(ID:String):TGUID;
   function  md5(s:string):string;

procedure Register;
function Prepared(token:String):String;
function strippunctuation(s:string):string;
function StripHTML(S: string): string;

function padleft(value:String;size:integer):String;

implementation

const
 GUID_NULL: TGUID = '{00000000-0000-0000-0000-000000000000}';

{$R *.dcr}

procedure Register;
begin
  RegisterComponents('VSDB',[TvsdbSDKClient]);
end;

function StripHTML(S: string): string;
var
  i,max:integer; intag:boolean;
begin
  max:=length(s);
  intag:=false; result:='';
  for i:= 1 to max do begin
     if s[i]='<' then intag:=true
     else
     if s[i]='>' then begin intag:=false; result:=result+' '; end
     else
     if not intag then result:=result+s[i];
  end;
end;

function strippunctuation(s:string):string;
var
  i,max:integer; intag:boolean;
begin
  max:=length(s);
  intag:=false; result:='';
  for i:= 1 to max do begin
      if not(s[i] in ['%','&','+','’','_','#','$','@','/','\','|','&',',','.','"','?',')','(','{','}',']','[','''',':',';','!','-'] )then result:=result+s[i] else result:=result+' ';
  end;
end;

function md5(s:String):string;
begin
  result:=md5print(md5string(s));
end;

function Prepared(token:String):String;
var prep:string;
begin
  prep:=PorterStem(lowercase(trim(strippunctuation(token))));
  result:='';
if trim(prep)<>'' then
  result:=trim(md5print(md5string(prep)));
end;

procedure Log(msg:string);
//var r:tstringlist;
begin
{
 r:=tstringlist.create;
 r.LoadFromFile('log.txt');
 r.add(msg);
 r.SaveToFile('log.txt');
 r.free;
 }
end;


function TvsdbSDKClient.Codelist(Text:string):tstringlist;
var encoded:tstringlist; i,max:integer;   s,st:string;
begin
 encoded:=tstringlist.create;
 result:=tstringlist.Create;
 encoded.Delimiter:=' ';
 encoded.DelimitedText := striphtml( trim(strippunctuation(text)));
 max:=encoded.count-1;
 for i:= 0 to max do begin
   st:=prepared(encoded[i]);
   if st<>'' then result.add(st+'='+strippunctuation(encoded[i]));
 end;
 encoded.free;
end;

function TvsdbSDKClient.RawEncoded(Text:string):string;
var encoded:tstringlist; i,max:integer;   s,st,stop,encode:string;
begin
 encoded:=tstringlist.create;
 result:='';
 encoded.Delimiter:=' ';
 encoded.DelimitedText := striphtml( trim(strippunctuation(text)));
 max:=encoded.count-1;
 for i:= 0 to max do begin
   st:=PorterStem(lowercase(trim(strippunctuation(encoded[i]))));
   stop:=prepared(encoded[i]);
   encode := trim(encoded[i]);
   if st<>'' then
     if stoplist.IndexOf(encode)=-1 then
       if stoplist.IndexOf(stop)=-1 then
         result:=result+strippunctuation(st)+' ';
 end;
 encoded.free;
 result:=trim(result);
end;


constructor TvsdbSDKClient.create(Aowner:TComponent);
var id:tguid; i,max:integer;  s:String;
begin
 inherited create(aowner);
  fmessage:=tstringlist.create;
 if not (csDesigning in ComponentSTate) then  vsdb:=create_cloudspace;

  createguid(id);
  sessionid := GuidToID(id);

  fstoplist:=tstringlist.Create;
  fstoplist.add(prepared('an'));
  fstoplist.add(prepared('br'));fstoplist.add(prepared('I'));
    fstoplist.add(prepared('nbsp'));
    fstoplist.add(prepared('quot'));
    fstoplist.add(prepared('strong'));
    fstoplist.add(prepared('em'));
    fstoplist.add(prepared('p')); fstoplist.add(prepared('also'));
    fstoplist.add(prepared('com'));fstoplist.add(prepared('us'));fstoplist.add(prepared('www'));
    fstoplist.add(prepared('org')); fstoplist.add(prepared('gov'));
    fstoplist.add(prepared('a'));fstoplist.add(prepared('to'));
    fstoplist.add(prepared('the'));    fstoplist.add(prepared('of'));
    fstoplist.add(prepared('with'));    fstoplist.add(prepared('by'));
    fstoplist.add(prepared('for'));     fstoplist.add(prepared('on'));
     fstoplist.add(prepared('am'));     fstoplist.add(prepared('which'));
   fstoplist.add(prepared('few'));     fstoplist.add(prepared('all'));
  fstoplist.add(prepared('them'));     fstoplist.add(prepared('to'));
  fstoplist.add(prepared('we'));
  fstoplist.add(prepared('me'));     fstoplist.add(prepared('but'));
   fstoplist.add(prepared('it'));     fstoplist.add(prepared('is'));fstoplist.add(prepared('was'));
   fstoplist.add(prepared('as'));     fstoplist.add(prepared('my'));
   fstoplist.add(prepared('if'));  fstoplist.add(prepared('and'));
   fstoplist.add(prepared('in'));  fstoplist.add(prepared('that'));
   fstoplist.add(prepared('you')); fstoplist.add(prepared('this'));
   fstoplist.add(prepared('have')); fstoplist.add(prepared('has'));
   fstoplist.add(prepared('had'));fstoplist.add(prepared('be'));
   fstoplist.add(prepared('can')); fstoplist.add(prepared('are'));
   fstoplist.add(prepared('would')); fstoplist.add(prepared('shall'));
   fstoplist.add(prepared('could')); fstoplist.add(prepared('hi'));
   fstoplist.add(prepared('thanks'));
   fstoplist.add(prepared('thanx')); fstoplist.add(prepared('teh'));
   fstoplist.add(prepared('what')); fstoplist.add(prepared('do'));
   fstoplist.add(prepared('or'));  fstoplist.add(prepared('they'));
   fstoplist.add(prepared('like')); fstoplist.add(prepared('can'));
  fstoplist.add(prepared('not'));fstoplist.add(prepared('at'));
  fstoplist.add(prepared('so')); fstoplist.add(prepared('would'));
  fstoplist.add(prepared('some'));fstoplist.add(prepared('how'));
  fstoplist.add(prepared('your'));fstoplist.add(prepared('from'));fstoplist.add(prepared('we'));
  fstoplist.add(prepared('there'));fstoplist.add(prepared('here'));
  fstoplist.add(prepared('out')); fstoplist.add(prepared('get')); fstoplist.add(prepared('our'));
  fstoplist.add(prepared('up')); fstoplist.add(prepared('been'));fstoplist.add(prepared('their'));
  fstoplist.add(prepared('these'));  fstoplist.add(prepared('those'));
  fstoplist.add(prepared('now'));  fstoplist.add(prepared('much'));
  fstoplist.add(prepared('amp'));
  fstoplist.add(prepared('who')); fstoplist.add(prepared('when'));  fstoplist.add(prepared('http'));
  fstoplist.add(prepared('html')); fstoplist.add(prepared('href'));
  

 
end;

destructor TvsdbSDKClient.destroy;
begin
 try
// if not (csDesigning in ComponentSTate) then destroy_cloudspace(@VSDB);
 except on E:Exception do
 end;
 fmessage.free;
 fstoplist.free;
 inherited destroy;
end;

function  GuidToID(ID:TGUID):string;
var sid:string ;
begin
 sid:=GuidToString(id);
 sid:=stringReplace(sid,'{','',[rfReplaceAll]);
 sid:=stringReplace(sid,'}','',[rfReplaceAll]);
 sid:=stringReplace(sid,'-','',[rfReplaceAll]);
 sid:=lowercase(sid);
 result:=sid;
end;

function  IDToGUID(ID:String):TGUID;
var hold:String;
begin
 hold := '{'+copy(id,1,8) + '-'+copy(id,9,4)+'-'+copy(id,13,4)+'-'+copy(id,17,4)+'-'+copy(id,21,12)+'}';
 result:=Stringtoguid(uppercase(hold));
end;

function  TvsdbSDKClient.CreateID:string;
var id:TGUID;
begin
 createguid(id);
 result:=GuidToID(id);
end;

function  TvsdbSDKClient.NullID:string;
const GUID_NULL : TGUID =  '{00000000-0000-0000-0000-000000000000}';
begin
 result:=GuidToID(GUID_NULL);
end;



function padleft(value:String;size:integer):String;
var sz:integer;
begin
  result:=value;
  sz:=length(result);
  while (sz<size) do begin result:='0'+result;  inc(sz); end;
end;

function TvsdbSDKClient.CreateMessage(Command:char;ID:String;value:single):string;
var v:string;
begin
 if round(value)=value then
      v:=inttostr(round(value))
    else
      v:=floattostr(value);
 if length(v)<10 then v:=padleft(v,10);
 if length(v)>10 then v:=copy(v,1,10);
 result:= command +' '+ID+' '+v;
end;

procedure TvsdbSDKClient.DeleteVector(DocumentID:string);
begin
 fmessage.add(CreateMessage('-',DocumentID,0));
end;

procedure TvsdbSDKClient.BeginVector(VectorID:String;Dimensions:integer);
begin
 fmessage.add(CreateMessage('V',VectorID,Dimensions));
end;

procedure TvsdbSDKClient.BeginQuery(QueryID:String;Dimensions:integer);
begin
 fmessage.add(CreateMessage('Q',QueryID,Dimensions));
end;

procedure TvsdbSDKClient.AddDimension(DimensionID:String;Value:single);
begin
  fmessage.add(CreateMessage('D',DimensionID,Value));
end;

procedure TvsdbSDKClient.Rollback;
begin
 fmessage.Clear;
end;

procedure TvsdbSDKClient.Commit;
begin
 SendMessage(fmessage.text,false,nil);
 fmessage.Clear;
end;

procedure TvsdbSDKClient.SendQuery(QueryID:string;Threshold:single;resultlist:tstrings); {results come back DocumentID=Score}
begin
 fmessage.add(CreateMessage('E',QueryID,Threshold));
 SendMessage(fmessage.text,true,resultlist);
 fmessage.Clear;
end;



procedure TvsdbSDKClient.SendMessage(Msg:String;expectReply:boolean;Results:TStrings);
var seg:string;Hmsg,line,element :string;command:char;value:single;  sz,i,max:integer; m:tstringlist;
    buffer:pchar;
begin
buffer:=allocmem(512);
if not busy then begin
try
  busy:=true;
   try
     hmsg:=msg;
     log(msg);

     m:=tstringlist.create;
     m.text:=msg;
     max:=m.count-1;
     for i:= 0 to max do begin
       line:=trim(m[i]);
       if line<>'' then
       if length(line)>32 then begin
       try
       command:=line[1];
       system.delete(line,1,2);
       element:=copy(line,1,32);
       system.delete(line,1,32);
       value:=strtofloat(trim(line));
       except on E:Exception do

       end;
       end;
       vsdb_execute(VSDB,command,pchar(element),value);
     end;
     m.free;

    sz:= vsdb_outputbuffer(vsdb,buffer);
    while sz>0 do begin
      results.Add(buffer);
      fillchar(buffer[0],512,0);
      sz:= vsdb_outputbuffer(vsdb,buffer);
    end;

   except on E:Exception do
      // raise(Exception.Create('Unable to process message') );
   end; //except
finally
 begin
  busy:=false;
  system.FreeMem (buffer);
 end;
end; //finally
end; // if busy
end;

procedure TvsdbSDKClient.RefreshCache;
begin
  SendMessage(CreateMessage('R',NullID(),0)+#13+#10,false,nil);
  fmessage.clear;
end;

function TvsdbSDKClient.InStopList(token:String):boolean;
begin
 result:=(fstoplist.indexof(token)>-1);
end;

function TvsdbSDKClient.RawInStopList(token:String):boolean;
begin
 result:=(fstoplist.indexof(prepared(token))>-1);
end;



procedure TvsdbSDKClient.EncodeText(text: string;Vector:tstringlist);
var i,max,cpos,cmax:integer; token,hold,vs,seg:string; tokens,safetokens:TStringlist;
   vcount:integer;
begin
 tokens:=TStringlist.create;  safetokens:=TStringlist.create;
 //tokens.Delimiter := ' ';
 //tokens.DelimitedText := trim(strippunctuation( Text)) ;

 hold:= trim(strippunctuation( Text)) ;
 cmax:= length(hold);
 for cpos:= 1 to cmax do begin
   if hold[cpos]<>' ' then
      seg:=seg+hold[cpos]
      else
        if trim(seg)<>'' then begin tokens.add(seg); seg:='' end;
 end;
 if trim(seg)<>'' then begin tokens.add(seg); seg:='' end;


  max:= tokens.count-1;
  for i:= 0 to max do begin
    token:=prepared(tokens[i]);
   if token<>'' then
   if not InStopList( token) then begin
     safetokens.Add(tokens[i]);
     vs:=vector.Values[token];
     if vs='' then vcount:=0 else vcount:= strtoint(vs);
     if vcount=0 then begin
        Vector.add(token+'=1');
        end
     else vector.values[token]:=inttostr(vcount+1);
  end;
  end;
  tokens.free;

  {
 max:=safetokens.count-1;
 for i:= 1 to max-1 do begin
   hold:=safetokens[i-1] + ' ' + safetokens[i];
   token:=prepared(hold);
   vs:=vector.Values[token];
     if vs='' then vcount:=0 else vcount:= strtoint(vs);
     if vcount=0 then begin
        Vector.add(token+'=1');
        end
     else vector.values[token]:=inttostr(vcount+1);
 end;
 }
 safetokens.free;

end;


procedure TvsdbSDKClient.Replace(DocumentID:TGUID;Text:String);
var encoded:tstringlist; i,max:integer;
begin
 encoded:=tstringlist.create;
 EncodeText(StripHTML(Text),encoded);
// encoded.savetofile('replace_'+GuidToID(documentid)+'.txt');
 Self.BeginVector(GuidToID(documentid),encoded.count);
 max:=encoded.count-1;
 for i:= 0 to max do begin
   AddDimension( encoded.names[i],strtofloat(encoded.ValueFromIndex[i]));
 end;
 commit;
 encoded.free;
end;


function TvsdbSDKClient.RawScript(DocumentID:TGUID;Text:String):string;
var encoded:tstringlist; i,max:integer;
begin
 encoded:=tstringlist.create;
 EncodeText(StripHTML(Text),encoded);
// encoded.savetofile('replace_'+GuidToID(documentid)+'.txt');
 Self.BeginVector(GuidToID(documentid),encoded.count);
 max:=encoded.count-1;
 for i:= 0 to max do begin
   AddDimension( encoded.names[i],strtofloat(encoded.ValueFromIndex[i]));
 end;
 result:= fmessage.Text;
  fmessage.Clear;
 encoded.free;
end;

procedure TvsdbSDKClient.ReplaceNameValues(DocumentID:TGUID;data:tstringlist);
var encoded:tstringlist; i,max:integer;  n,v:string;
begin
 encoded:=tstringlist.create;
 max:=data.count-1;
 for i:= 0 to max do begin
   n:=data.Names[i];
   v:=data.Values[n];
   if trim(v)='' then v:='0';
   encoded.Add(n+'='+v);
 end;
 Self.BeginVector(GuidToID(documentid),encoded.count);
 max:=encoded.count-1;
 for i:= 0 to max do begin
   AddDimension( encoded.names[i],strtofloat(encoded.ValueFromIndex[i]));
 end;
 commit;
 encoded.free;
end;


procedure TvsdbSDKClient.test_StartQuery(DocumentID:TGUID;Text:String);
var i,max:integer;
begin
 test_encoded:=tstringlist.create;
 EncodeText(Text,test_encoded);
// test_encoded.savetofile('encode.txt');
 Self.BeginVector(GuidToID(documentid),test_encoded.count);
 max:=test_encoded.count-1;
 for i:= 0 to max-1 do begin
   AddDimension( test_encoded.names[i] ,strtofloat(test_encoded.ValueFromIndex[i]));
 end;
 SendMessage(fmessage.text,false,nil);
 test_encoded.Free;
end;

procedure TvsdbSDKClient.test_CompleteQuery;
var max:integer;
begin
 max:=test_encoded.count-1;
 AddDimension( test_encoded.names[max] ,strtofloat(test_encoded.ValueFromIndex[max]));
 test_encoded.free;
end;

procedure TvsdbSDKClient.Query(QueryID:TGUID;Threshold:single;Text:String;Results:TStrings);
var encoded,holdresults,parseline:tstringlist; i,max:integer;
begin
 encoded:=tstringlist.create;   holdresults:=tstringlist.create; parseline:=tstringlist.create;
 EncodeText(Text,encoded);
// encoded.savetofile('query.txt');
if encoded.count>0 then begin 
 Self.BeginQuery(GuidToID(QueryID),encoded.count);
 max:=encoded.count-1;
 for i:= 0 to max do begin
   AddDimension( encoded.names[i] ,strtofloat(encoded.ValueFromIndex[i]));
 end;
   self.SendQuery (GuidToID(QueryID),Threshold,HoldResults);
   parseline.Delimiter :=' ';
   max:=holdresults.count-1;
   for i:= 0 to max do begin
      parseline.DelimitedText:=holdresults[i];
      if parseline[0]='R' then results.Add(parseline[1]+'='+parseline[4]);
   end;
 end else results.add('Q 0 DONE');
 encoded.free;  holdresults.Free; parseline.free;
end;

procedure TvsdbSDKClient.Delete(DocumentID:TGUID);
begin
 DeleteVector(GuidToID(DocumentID));
 Commit;
end;
 

procedure TvsdbSDKClient.LoadFromFile(filename:string);
var results:tstringlist;max,i:integer;line, id,val: string;  c:char;
begin
if fileexists(filename) then begin
 results:=tstringlist.Create;
 results.LoadFromFile(filename);
 max:=results.count-1;
 for i:= 0 to max do begin
  line:=trim(results[i]);
  if line<>'' then begin
   c:=line[1];
   system.delete(line,1,2);
   id:=copy(line,1,32);
   system.delete(line,1,33);
   val:=line;
   vsdb_execute(vsdb,c,pchar(id),strtofloat(val));
  end;
 end;
 results.free;
 end;
end;

procedure TvsdbSDKClient.SaveToFile(filename:string);
var sz:integer; buffer:pchar; results:tstringlist; f,t:string;
begin
//  buffer := allocmem(1024);
  f:=filename; t:=changefileext(filename,'_thesaurus.dat');
  vsdb_savecloudspace(vsdb,pchar(f),pchar(t));

  {results:=tstringlist.Create;

  vsdb_execute(vsdb,'S','00000000000000000000000000000000',0);
  sz:= vsdb_outputbuffer(vsdb,buffer);
  while sz>0 do begin
      results.Add(trim(buffer));
      fillchar(buffer[0],1024,0);
      sz:= vsdb_outputbuffer(vsdb,buffer);
  end;
  results.Delete(0);
  results.Delete(results.count-1);
  results.SaveToFile(filename);
  freemem(buffer); results.Free;
  }
end;


procedure TvsdbSDKClient.Mode(modecode:integer;thesaurus_threshold:single);
begin
 fmessage.Clear;
 fmessage.add(CreateMessage('M',padleft(inttostr(abs(modecode)),32),thesaurus_threshold));
 SendMessage(fmessage.text,false,nil);
 fmessage.Clear;
end;

procedure TvsdbSDKClient.Clusters(segment,segmentsize:integer;cluster_size_min:integer;threshold:single;Results:TStrings);
var sz:integer;buffer:pchar;
begin
    vsdb_metacluster(vsdb,segment,segmentsize,threshold,cluster_size_min);
    buffer:=allocmem(512);
    sz:= vsdb_outputbuffer(vsdb,buffer);
    while sz>0 do begin
      results.Add(buffer);
      fillchar(buffer[0],512,0);
      sz:= vsdb_outputbuffer(vsdb,buffer);
    end;
    freemem(buffer);
end;

procedure TvsdbSDKClient.Dump(s:tstrings);
begin
 fmessage.Clear;
 fmessage.add(CreateMessage('S',GUIDtoID(GUID_NULL),0));
 SendMessage(fmessage.text,true,s);
 fmessage.Clear;
end;

end.

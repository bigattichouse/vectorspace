unit vsdbClient;

interface
uses Classes,SysUtils, Graphics, Controls, IdBaseComponent, IdComponent,
  IdTCPConnection, IdTCPClient,uPorterStem,md5 ;

type
TvsdbClient = class(TComponent)
 private
   fserver:string; fport:integer;  busy:boolean;fstayconnected:boolean;
   fMessage:tstringlist;
   VSDB:TIdTCPClient; sessionid:string;
   function  CreateMessage(Command:char;ID:String;value:single):string;
 protected
   procedure BeginVector(VectorID:String;Dimensions:integer);
   procedure BeginQuery(QueryID:String;Dimensions:integer);
   procedure DeleteVector(DocumentID:String);
   procedure AddDimension(DimensionID:String;Value:single);
   procedure SendQuery(QueryID:string;Threshold:single;resultlist:tstrings); {results come back DocumentID=Score}
   procedure RefreshCache;
   function  CreateID:string;
   function  NullID:string;
   procedure SendMessage(Msg:String;expectReply:boolean;Results:TStrings);
   procedure Rollback;
   procedure Commit;
   procedure Connect;
 public
   fstoplist:tstringlist;
   constructor create(Aowner:TComponent);override;
   destructor destroy;override;           
   function InStopList(token:String):boolean;
   function RawInStopList(token:String):boolean;
   property Stoplist:tstringlist read fstoplist;    
   procedure EncodeText(text: string;Vector:tstringlist);
 published
   property  Server:String read fserver write fserver;
   property  port:integer read fport write fport;
   property  stayconnected:boolean read fstayconnected write fstayconnected;
   procedure Replace(DocumentID:TGUID;Text:String);
   procedure Query(QueryID:TGUID;Threshold:single;Text:String; Results:TStrings);
   procedure Delete(DocumentID:TGUID);
   procedure Reload;
 end;

TvsdbSocketClient = class(TvsdbClient);

   function  GuidToID(ID:TGUID):string;
   function  IDToGUID(ID:String):TGUID;  
   function md5(s:String):string;

procedure Register;

implementation

{$R *.dcr}

procedure Register;
begin
  RegisterComponents('VSDB',[TvsdbClient]);
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
  i,max:integer;  
begin
  max:=length(s);
  result:='';
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


function padleft(value:String;size:integer):String;
begin
  result:=value;
  while (length(result)<size) do result:='0'+result;
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

constructor TvsdbClient.create(Aowner:TComponent);
var id:tguid;
begin
 inherited create(aowner);
  fmessage:=tstringlist.create;
 if not (csDesigning in ComponentSTate) then
  VSDB:=TIdTCPClient.Create(nil);
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

destructor TvsdbClient.destroy;
begin
 try
 if not (csDesigning in ComponentSTate) then VSDB.Free;
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

function  TvsdbClient.CreateID:string;
var id:TGUID;
begin
 createguid(id);
 result:=GuidToID(id);
end;

function  TvsdbClient.NullID:string;
const GUID_NULL : TGUID =  '{00000000-0000-0000-0000-000000000000}';
begin
 result:=GuidToID(GUID_NULL);
end;

procedure  TvsdbClient.connect;
begin
 VSDB.Host := fserver;
 VSDB.Port := fport;
 VSDB.Connect;
end;

function TvsdbClient.CreateMessage(Command:char;ID:String;value:single):string;
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

procedure TvsdbClient.DeleteVector(DocumentID:string);
begin
 fmessage.add(CreateMessage('-',DocumentID,0));
end;

procedure TvsdbClient.BeginVector(VectorID:String;Dimensions:integer);
begin
 fmessage.add(CreateMessage('V',VectorID,Dimensions));
end;

procedure TvsdbClient.BeginQuery(QueryID:String;Dimensions:integer);
begin
 fmessage.add(CreateMessage('Q',QueryID,Dimensions));
end;

procedure TvsdbClient.AddDimension(DimensionID:String;Value:single);
begin
  fmessage.add(CreateMessage('D',DimensionID,Value));
end;

procedure TvsdbClient.Rollback;
begin
 fmessage.Clear;
end;

procedure TvsdbClient.Commit;
begin
 SendMessage(fmessage.text,false,nil);
 fmessage.Clear;
end;

procedure TvsdbClient.SendQuery(QueryID:string;Threshold:single;resultlist:tstrings); {results come back DocumentID=Score}
begin
 fmessage.add(CreateMessage('E',QueryID,Threshold));
 SendMessage(fmessage.text,true,resultlist);
 fmessage.Clear;
end;

procedure TvsdbClient.SendMessage(Msg:String;expectReply:boolean;Results:TStrings);
var seg:string;Hmsg:string;
begin
if not busy then begin
try
  busy:=true;
   try
     hmsg:=msg;
     log(msg);                 
      if not vsdb.connected then Connect;
        vsdb.WriteBuffer(hmsg[1],length(hmsg));
        if expectReply then begin
         seg :=  vsdb.ReadLn;
         if pos('DONE',seg)=0 then  results.add(seg);
         while ((pos('DONE',seg)=0) and (pos('BUSY',seg)=0) and (pos('ERROR',seg)=0))do begin
            seg :=  vsdb.ReadLn;
            if pos('DONE',seg)=0 then results.add(seg);
         end;
         end;
   except on E:Exception do
       raise(Exception.Create('Unable to send message to '+fserver) );
   end; //except
finally
  busy:=false;
end; //finally
end; // if busy
end;

procedure TvsdbClient.RefreshCache;
begin
  SendMessage(CreateMessage('R',NullID(),0)+#13+#10,false,nil);
  fmessage.clear;
end;


function TvsdbClient.InStopList(token:String):boolean;
begin
 result:=(fstoplist.indexof(token)>-1);
end;

function TvsdbClient.RawInStopList(token:String):boolean;
begin
 result:=(fstoplist.indexof(prepared(token))>-1);
end;
 

procedure TvsdbClient.EncodeText(text: string;Vector:tstringlist);
var i,max,cpos,cmax:integer; token,hold,vs,seg:string; tokens,safetokens:TStringlist;
   vcount:integer;
begin
 tokens:=TStringlist.create;  safetokens:=TStringlist.create;
 //tokens.Delimiter := ' ';
 //tokens.DelimitedText := trim(strippunctuation( Text)) ;
 text:=stripHTML(text);
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
 safetokens.free;
end;


procedure TvsdbClient.Replace(DocumentID:TGUID;Text:String);
var encoded:tstringlist; i,max:integer;
begin
 encoded:=tstringlist.create;
 EncodeText(StripHTML(Text),encoded); 
 Self.BeginVector(GuidToID(documentid),encoded.count);
 max:=encoded.count-1;
 for i:= 0 to max do begin
   AddDimension( encoded.names[i],strtofloat(encoded.ValueFromIndex[i]));
 end;
 commit;
 encoded.free;
end;




procedure TvsdbClient.Query(QueryID:TGUID;Threshold:single;Text:String;Results:TStrings);
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

procedure TvsdbClient.Delete(DocumentID:TGUID);
begin
 DeleteVector(GuidToID(DocumentID));
 Commit;
end;

procedure TvsdbClient.Reload;
begin
  refreshCache;
end;

end.

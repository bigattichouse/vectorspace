program clienttest;

uses
  Forms,
  testu in 'testu.pas' {Form1},
  uPorterStem in 'uPorterStem.pas',
  md5 in 'md5.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.

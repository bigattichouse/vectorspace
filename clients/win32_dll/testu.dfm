object Form1: TForm1
  Left = 192
  Top = 114
  Width = 602
  Height = 487
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Memo2: TMemo
    Left = 16
    Top = 48
    Width = 529
    Height = 361
    Lines.Strings = (
      'Memo2')
    TabOrder = 0
  end
  object Button1: TButton
    Left = 16
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Button1'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 408
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Button2'
    TabOrder = 2
    OnClick = Button2Click
  end
  object Edit1: TEdit
    Left = 144
    Top = 8
    Width = 249
    Height = 21
    TabOrder = 3
    Text = 'Edit1'
  end
  object client: TvsdbSDKClient
    Left = 96
    Top = 8
  end
end

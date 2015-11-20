<?
 include("vsdbclient.php");

 $conn = ConnectToVSDB("office","9999");
 for ($i=0;$i<2;$i++){
 $testload = array();
  AddClassDimension(&$testload,"GENDER",100);
  AddClassDimension(&$testload,"GENDERSEEK",-100);
  AddClassDimension(&$testload,"AGE",rand(30,50));
  UpdateVsdb (  md5(rand()),$testload);
 }
  RefreshVsdb ( );
DisconnectVSDB (&$conn);


for ($i=0;$i<1000;$i++){
  $conn = ConnectToVSDB("office","9999");
  DisconnectVSDB (&$conn);
}



?>

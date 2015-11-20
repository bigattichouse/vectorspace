<?
 include("vsdbclient.php");

 $conn = ConnectToVSDB("office","9997");
 for ($i=0;$i<2;$i++){
 $testload = array();
  AddClassDimension(&$testload,"GENDER",100);
  AddClassDimension(&$testload,"GENDERSEEK",-100);
  AddClassDimension(&$testload,"AGE",rand(30,50));
  UpdateVsdb (  md5(rand()),$testload);
 }
  RefreshVsdb ( );
DisconnectVSDB (&$conn);


for ($i=0;$i<100;$i++){
  $conn = ConnectToVSDB("office","9997");
  $query = array();
  AddClassDimension(&$query,"GENDER",100);
  AddClassDimension(&$query,"GENDERSEEK",-100);
  AddClassDimension(&$query,"AGE",31);
  echo QueryVsdb( md5(rand()),$query,0.5);
  //DisconnectVSDB (&$conn);
}



?>

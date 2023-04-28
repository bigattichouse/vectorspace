<?php
global $conn;

function padfloat ($value){
 $value=trim($value);
 $sign="";
 $sign = "0";
 if ($value[0]=="-"){$sign = "-"; $return = substr($value,1);} else {$return=$value;}
 while (strlen($return)<9){
   $return = "0" . $return;
 }
 return ($sign . $return);
}

function MSG_Query ($msg,$id,$value){
 $value = padfloat($value);
 return("$msg $id $value\r\n");
}



function ConnectToVSDB ($server,$port){
 //socket connection
$conn = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($conn < 0) {
echo "socket_create() failed: reason: " . socket_strerror($socket) . "\n";
}
$result = socket_connect($conn, $server, $port);
if ($result < 0) {
echo "socket_connect() failed.\nReason: ($result) " . socket_strerror($result) . "\n";
}
 return($conn);
}

function DisconnectVSDB ( ){
 global $conn;
 socket_close($conn);
}

function SendVsdb($msg){
 global $conn;
 echo $msg;
 socket_write($conn, $msg, strlen($msg));
}

function ReadVsdb(){
 global $conn;
 $results = "";
 while ($out = socket_read($conn, 2048)) {
  $results .= $out;   }
 return($results);
}

function QueryVsdb ( $docid,$dimarray,$threshold){
 global $conn;
 $msg = MSG_Query("Q",$docid,sizeof($dimarray));
 SendVsdb($msg);
 foreach($dimarray as $key=>$value){
    $msg = MSG_Query("D",$key,$value);
    SendVsdb($msg);
 }
 $msg .= MSG_Query("X",$docid,$threshold);
 SendVsdb($msg);

 $results=ReadVsdb();
 return ($results);
}

function UpdateVsdb (  $docid,$dimarray){
 global $conn;
 //update a vector in the db
 //assume we used my stemmer php stuff to handle the stemming, and have our array of key->value pairs
 $msg = MSG_Query("V",$docid,sizeof($dimarray));
 SendVsdb($msg);
 foreach($dimarray as $key=>$value){
    $msg = MSG_Query("D",$key,$value);
     SendVsdb($msg);
 }
}

function RefreshVsdb( ){
 global $conn;
 //refreshes the WHOLE CACHE, which will cause all clients to wait 1-3 seconds until its
 //reloaded.
 SendVsdb(MSG_Query("R","00000000000000000000000000000000",0));
}

function AddClassDimension(&$vector,$term,$value){
 $vector[md5("CLASS_" . $term)]=$value;
}

?>

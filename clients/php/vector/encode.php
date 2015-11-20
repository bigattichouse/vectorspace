<?
require("stemmer.php");

/************************************************************************
*                                                                       *
* encode.php                                                            *
*                                                                       *
*************************************************************************
*                                                                       *
* Vector Term Space Search Engine                                       *
*                                                                       *
* Copyright (c) 2003 BigAtticHouse <www.bigattichouse.com>              *
* All rights reserved.                                                  *
*                                                                       *
* This script is free software; you can redistribute it and/or modify   *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* The GNU General Public License can be found at                        *
* http://www.gnu.org/copyleft/gpl.html.                                 *
*                                                                       *
* This script is distributed in the hope that it will be useful,        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
* GNU General Public License for more details.                          *
*                                                                       *
* Author(s): Michael Johnson <father@bigattichouse.com>                 *
*                                                                       *
* Last modified: 08/28/03                                               *
*                                                                       *
*************************************************************************/

$stoplist = array();

function BuildStopListArray(){
  global $db;
  global $stops;
  $SQL="Select TERM FROM STOPWORDS order by TERM";
  $fetch=mysql_query($SQL,$db);
  while ($row =mysql_fetch_row($fetch)){
   $stoplist[$row[0]]="1";
  }
}

function TermSpaceSize(){
  return(FetchValue("select count(*) from TERMSPACE"));
}

function AssureTerm($stemmed){
  global $stoplist;
  global $db;

  if ($stoplist[$stemmed]==""){
  $ID= fetchValue("select DIMENSION FROM TERMS WHERE TERM='$stemmed'");
  if ($ID==""){
   $ID=CreateGUID();
   $stemmed = addslashes($stemmed);
   //mysql_query("Lock Tables TERMS",$db) or die(mysql_error());
   ExecSQL("INSERT INTO TERMS (DIMENSION,TERM) VALUES ('$ID','$stemmed')");
   //ExecSQL("UNLOCK TABLES");
  }
  }
}

function CountTerms($TERM,$ARR){
 $result=0;
 for ($i=0;$i<sizeof($ARR);$i++){
   if ($ARR[$i]==$TERM){$result+=1;}
 }
 return($result);
}

function FindTerms($line){
 //get term and remove from line
 $inquote=False; $counter=0;
 $terms=array();
 for($i=0;$i<strlen($line);$i++){
  $c=substr($line,$i,1);
  if($c=='"'){$inquote = !$inquote;}
  if (($c==' ') and (!$inquote)){
    //$line = substr($line,strlen($seg)+1);
    $terms[$counter]=$seg;
    $seg=""; $counter+=1;
    } else { $seg.= $c;}
 }
  if ($seg!=""){$terms[$counter]=$seg;}
  return($terms);
}


function EncodeVectorSpace($DOCUMENT){
 global $db;
 //strip html
 $DOCUMENT = strip_tags($DOCUMENT);
 //$unstemmed = split(" ",$DOCUMENT);
 
 $i=0;
 $unstemmed=FindTerms($DOCUMENT);  
 //print_r($unstemmed);
 //stem the doc
 $c=0; $stemmed = array();
 $stm = new Stemmer();
 for ($i=0;$i<sizeof($unstemmed);$i++){
                $unstemmed[$i]=trim(strtolower($unstemmed[$i]));
                $string = $unstemmed[$i];
                $string = ereg_replace("^[:alnum:]", "", $string);
		//echo $string;
                $stemmed[$c] = $stm->stem($string);
                //echo $stemmed[$c];
                AssureTerm($stemmed[$c]);
                $c+=1;
 }

 $sz = sizeof($stemmed);

 for ($i=0;$i<$sz;$i++){
   if ($i>0){
        $doublet = $stemmed[($i-1)] . '-' . $stemmed[($i)];
        AssureTerm($doublet);
        $stemmed[$c]=$doublet;
        $c+=1;
        }             
   if ($i<$sz){
        $doublet = $stemmed[($i)] . '-' . $stemmed[($i+1)];
        AssureTerm($doublet);
        $stemmed[$c]=$doublet;
        $c+=1;
    }
 }

 //encode vector, ignoring stop words.
 $vector = array();
 $SQL = "SELECT DIMENSION,TERM FROM TERMS ORDER BY TERM";
 $fetch = mysql_query($SQL,$db);
 $i=0;
 while ($row = mysql_fetch_row($fetch)){
   if(($c=CountTerms($row[1],$stemmed))>0){
    $vector[$i]= array();
    $vector[$i][0] = $row[0];
    $vector[$i][1] = $c;
    $i+=1;
   }
 }
 mysql_free_result($fetch);
 //now the vector is ready, containing dimensions & counts>0
 //you can build an insert by using TERM_[DIMENSION]=[Count]
 return($vector);
}

function StoreDocumentVector ($DOCUMENTID,$vector){
 global $db;
 $imax = sizeof($vector);
 for ($i=0;$i<$imax;$i++){
   $SQL="REPLACE INTO VECTORS (DOCUMENTID,DIMENSION,COUNT)";
   $SQL .= "VALUES ('$DOCUMENTID'";
   $SQL .= ",'" . $vector[$i][0] . "'";
   $SQL .= "," . $vector[$i][1];
   $SQL.=")";
   ExecSQL($SQL);
 }
 RETURN($DOCUMENTID);
}


function StoreQUERYVector ($vector){
 global $db;
 $imax = sizeof($vector);
 $ID = CreateGUID();

 for ($i=0;$i<$imax;$i++){
   $SQL="REPLACE INTO QUERIES (QUERYID,DIMENSION,COUNT)";
   $SQL .= "VALUES ('$ID'";
   $SQL .= ",'" . $vector[$i][0] . "'";
   $SQL .= "," . $vector[$i][1];
   $SQL.=")";
   ExecSQL($SQL);
 }
 return ($ID);
}



?>

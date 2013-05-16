<?php

$xml = simplexml_load_string('<xml></xml>');

$str = "abc & def" ;

$xml["a1"] = "" ;
$xml["a1"] = htmlspecialchars($str,ENT_NOQUOTES) ;

$xml["a2"] = htmlspecialchars($str,ENT_NOQUOTES) ;

$xml["a3"] = "" ;
$xml["a3"] = $str ;

$xml["a4"] = $str ;

echo $xml->asXML();
?>
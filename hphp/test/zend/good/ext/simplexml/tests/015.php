<?php 
$xml =<<<EOF
<people>
   <person name="Joe"></person>
</people>
EOF;

$xml2 =<<<EOF
<people>
   <person name="Joe"></person>
   <person name="Boe"></person>
</people>
EOF;

$people = simplexml_load_string($xml);
var_dump($people->person['name']);
var_dump($people->person[0]['name']);
//$people->person['name'] = "XXX";
//var_dump($people->person['name']);
//var_dump($people->person[0]['name']);
//$people->person[0]['name'] = "YYY";
//var_dump($people->person['name']);
//var_dump($people->person[0]['name']);
//unset($people->person[0]['name']);
//var_dump($people->person['name']);
//var_dump($people->person[0]['name']);
//var_dump(isset($people->person['name']));
//var_dump(isset($people->person[0]['name']));
$people = simplexml_load_string($xml2);
var_dump($people->person[0]['name']);
var_dump($people->person[1]['name']);
?>
===DONE===

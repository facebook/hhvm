<?php 
$xml =<<<EOF
<people>
   <person name="Joe"></person>
</people>
EOF;

$people = simplexml_load_string($xml);
var_dump($people->person['name']);
$people->person['name'] = $people->person['name'] . 'Foo';
var_dump($people->person['name']);
$people->person['name'] .= 'Bar';
var_dump($people->person['name']);

echo "---[0]---\n";

$people = simplexml_load_string($xml);
var_dump($people->person[0]['name']);
$people->person[0]['name'] = $people->person[0]['name'] . 'Foo';
var_dump($people->person[0]['name']);
$people->person[0]['name'] .= 'Bar';
var_dump($people->person[0]['name']);

?>
===DONE===
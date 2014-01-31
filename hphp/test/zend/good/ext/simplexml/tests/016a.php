<?php 
$xml =<<<EOF
<people>
   <person name="Foo"></person>
</people>
EOF;

$people = simplexml_load_string($xml);
var_dump($people->person['name']);
$people->person['name'] .= 'Bar';
var_dump($people->person['name']);

?>
===DONE===
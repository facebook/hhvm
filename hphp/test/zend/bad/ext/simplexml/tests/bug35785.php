<?php

$xml = simplexml_load_string("<root></root>");
$xml->bla->posts->name = "FooBar";
echo $xml->asXML();
$xml = simplexml_load_string("<root></root>");
$count = count($xml->bla->posts);
var_dump($count);
$xml->bla->posts[$count]->name = "FooBar";
echo $xml->asXML();
$xml = simplexml_load_string("<root></root>");
$xml->bla->posts[]->name = "FooBar";
echo $xml->asXML();
?>
===DONE===
<?php exit(0); __halt_compiler(); ?>
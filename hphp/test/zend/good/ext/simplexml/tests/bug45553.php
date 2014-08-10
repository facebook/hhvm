<?php
$xml =<<<XML
<xml xmlns:a="http://a">
    <data a:label="I am A" label="I am Nothing">test1</data>
    <a:data a:label="I am a:A" label="I am a:Nothing">test2</a:data>
</xml>
XML;

$x = simplexml_load_string($xml);
$x->registerXPathNamespace("a", "http://a");

$atts = $x->xpath("/xml/data/@a:label");
echo $atts[0] . "\n";
$atts = $x->xpath("/xml/a:data");
echo $atts[0]->attributes() . "\n";
$atts = $x->xpath("/xml/a:data/@a:label");
echo $atts[0] . "\n";
$atts = $x->xpath("/xml/a:data/@label");
echo $atts[0] . "\n";
$atts = $x->xpath("/xml/data/@label");
echo $atts[0] . "\n";
?>
===DONE===

<?php


<<__EntryPoint>>
function main_text_node_offsetset() {
$xml = <<<XML
<test>
  <tag>text</tag>
</test>
XML;

$test = simplexml_load_string($xml);

$test->tag['foo'] = 'bar';
var_dump($test->tag['foo']);
}

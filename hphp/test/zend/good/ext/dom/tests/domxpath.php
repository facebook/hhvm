<?hh

function MyAverage($nodelist) :mixed{
    $count = 0;
    $val = 0;
    foreach ($nodelist as $node) {
        $count++;
        $val += HH\Lib\Legacy_FIXME\cast_for_arithmetic($node->textContent);
    }
    if ($val > 0) {
        return $val/$count;
    } else {
        return 0;
    }
}
<<__EntryPoint>> function main(): void {
require_once("dom_test.inc");
$dom = new DOMDocument;
$dom->loadXML(b'<root xmlns="urn::default"><child>myval</child></root>');

$xpath = new DOMXPath($dom);

$xpath->registerPHPFunctions(MyAverage<>);
$xpath->registerNamespace("php", "http://php.net/xpath");

$xpath->registerNamespace("def", "urn::default");
$nodelist = $xpath->query("//def:child");
if ($node = $nodelist->item(0)) {
    print $node->textContent."\n";
}

$count = $xpath->evaluate("count(//def:child)");

var_dump($count);

$xpathdoc = $xpath->document;

var_dump($xpathdoc is DOMDocument);

$root = $dom->documentElement;
$root->appendChild($dom->createElementNS("urn::default", "testnode", '3'));
$root->appendChild($dom->createElementNS("urn::default", "testnode", '4'));
$root->appendChild($dom->createElementNS("urn::default", "testnode", '4'));
$root->appendChild($dom->createElementNS("urn::default", "testnode", '5'));

$avg = $xpath->evaluate('number(php:function("MyAverage", //def:testnode))');
var_dump($avg);
}

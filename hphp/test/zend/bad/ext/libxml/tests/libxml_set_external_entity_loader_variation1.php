<?php
chdir(__DIR__);
$xml = <<<XML
<!DOCTYPE foo PUBLIC "-//FOO/BAR" "http://example.com/foobar">
<foo>bar&fooz;</foo>
XML;

$dtd = <<<DTD
<!ELEMENT foo (#PCDATA)>
<!ENTITY % fooentity PUBLIC
   "-//FOO/ENTITY"
   "fooentity.ent">
%fooentity;
DTD;

$entity = <<<ENT
<!ENTITY fooz "baz">
ENT;

libxml_set_external_entity_loader(
	function ($public, $system, $context) use($dtd,$entity){
		static $first = true;
		var_dump($public);
		var_dump($system);
		var_dump($context);
		$f = fopen("php://temp", "r+");
		fwrite($f, $first ? $dtd : $entity);
		$first = false;
		rewind($f);
		return $f;
	}
);

$dd = new DOMDocument;
$dd->resolveExternals = true;
$r = $dd->loadXML($xml);
var_dump($dd->validate());

echo "Done.\n";

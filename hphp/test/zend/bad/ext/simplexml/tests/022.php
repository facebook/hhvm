<?php 

$xml =<<<EOF
<?xml version='1.0'?>
<pres><content><file glob="slide_*.xml"/></content></pres>
EOF;

$sxe = simplexml_load_string($xml);

echo "===CONTENT===\n";
var_dump($sxe->content);

echo "===FILE===\n";
var_dump($sxe->content->file);

echo "===FOREACH===\n";
foreach($sxe->content->file as $file)
{
	var_dump($file);
	var_dump($file['glob']);
}

?>
===DONE===
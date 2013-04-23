<?php

$xml =<<<EOF
<?xml version='1.0'?>
<root>
<elem attr1='11' attr2='12' attr3='13'/>
<elem attr1='21' attr2='22' attr3='23'/>
<elem attr1='31' attr2='32' attr3='33'/>
</root>
EOF;

$sxe = simplexml_load_string($xml);

function test($xpath)
{
	global $sxe;

	echo "===$xpath===\n";
	var_dump($sxe->xpath($xpath));
}

test('elem/@attr2');
test('//@attr2');
test('//@*');
test('elem[2]/@attr2');

?>
===DONE===
<?php 

$sxe = simplexml_load_file(dirname(__FILE__).'/000.xml');

function test($what)
{
	global $sxe;
	echo "===$what\n";
	eval("var_dump(isset(\$$what));");
	eval("var_dump((bool)\$$what);");
	eval("var_dump(count(\$$what));");
	eval("var_dump(\$$what);");
}

test('sxe');
test('sxe->elem1');
test('sxe->elem1[0]');
test('sxe->elem1[0]->elem2');
test('sxe->elem1[0]->elem2->bla');
if (!ini_get("unicode_semantics")) test('sxe->elem1[0]["attr1"]');
test('sxe->elem1[0]->attr1');
test('sxe->elem1[1]');
test('sxe->elem1[2]');
test('sxe->elem11');
test('sxe->elem11->elem111');
test('sxe->elem11->elem111->elem1111');
test('sxe->elem22');
test('sxe->elem22->elem222');
test('sxe->elem22->attr22');
test('sxe->elem22["attr22"]');

?>
===DONE===
<?php exit(0); ?>
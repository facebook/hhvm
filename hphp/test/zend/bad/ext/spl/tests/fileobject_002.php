<?php

function test($name)
{
	echo "===$name===\n";

	$o = new SplFileObject(dirname(__FILE__) . '/' . $name);

	var_dump($o->key());
	while(($c = $o->fgetc()) !== false)
	{
		var_dump($o->key(), $c, $o->eof());
	}
	echo "===EOF?===\n";
	var_dump($o->eof());
	var_dump($o->key());
	var_dump($o->eof());
}

test('fileobject_001a.txt');
test('fileobject_001b.txt');

?>
===DONE===
<?php exit(0); ?>
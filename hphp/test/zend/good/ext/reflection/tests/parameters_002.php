<?php

function test($nix, Array $ar, &$ref, stdClass $std, NonExistingClass $na, stdClass &$opt = NULL, $def = "FooBar")
{
}

class test
{
	function test($nix, Array $ar, &$ref, stdClass $std, NonExistingClass $na, stdClass $opt = NULL, $def = "FooBar")
	{
	}
}

function check_params_decl_func($r, $f)
{
	$c = $r->$f();
	echo $f . ': ' . ($c ? ($c instanceof ReflectionMethod ? $c->class . '::' : '') . $c->name : 'NULL') . "()\n";
}

function check_params_decl_class($r, $f)
{
	$c = $r->$f();
	echo $f . ': ' . ($c ? $c->name : 'NULL') . "\n";
}

function check_params_func($r, $f)
{
	echo $f . ': ';
	$v = $r->$f();
	var_dump($v);
}

function check_params($r)
{
	echo "#####" . ($r instanceof ReflectionMethod ? $r->class . '::' : '') . $r->name . "()#####\n";
	$i = 0;
	foreach($r->getParameters() as $p)
	{
		echo "===" . $i . "===\n";
		$i++;
		check_params_func($p, 'getName');
		check_params_func($p, 'isPassedByReference');
		try
		{
			check_params_decl_class($p, 'getClass');
		}
		catch(ReflectionException $e)
		{
			echo $e->getMessage() . "\n";
		}
		check_params_decl_class($p, 'getDeclaringClass');
//		check_params_decl_func($p, 'getDeclaringFunction');
		check_params_func($p, 'isArray');
		check_params_func($p, 'allowsNull');
		check_params_func($p, 'isOptional');
		check_params_func($p, 'isDefaultValueAvailable');
		if ($p->isOptional())
		{
			check_params_func($p, 'getDefaultValue');
		}
	}
}

check_params(new ReflectionFunction('test'));

check_params(new ReflectionMethod('test::test'));

?>
===DONE===
<?php exit(0); ?>

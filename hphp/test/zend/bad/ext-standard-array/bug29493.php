<?php
function t1()
{
	$a = array('foo' => 'aaa');
	// refcount($a) = 1
	// refcount($a['foo']) = 1
	$b = $a;
	// refcount($a) = 2
	// refcount($a['foo']) = 1
	$b['foo'] = 'bbb';
	// refcount($a) = 1
	// refcount($a['foo']) = 1

	var_dump($a, $b);

	extract($a, EXTR_REFS);

	$foo = 'noo';

	var_dump($a, $b);
}

function t2()
{
	$a = array('foo' => 'aaa');
	// refcount($a) = 1
	// refcount($a['foo']) = 1
	$b = &$a;
	// refcount($a) = 2
	// is_ref($a) = true
	// refcount($a['foo']) = 1
	$b['foo'] = 'bbb';
	// refcount($a) = 2
	// refcount($a['foo']) = 1

	var_dump($a, $b);

	extract($a, EXTR_REFS);

	$foo = 'noo';

	var_dump($a, $b);
}

function t3()
{
	$a = array('foo' => 'aaa');
	// refcount($a) = 1
	// refcount($a['foo']) = 1
	$b = &$a;
	// refcount($a) = 2
	// is_ref($a) = true
	// refcount($a['foo']) = 1
	unset($b);
	// refcount($a) = 1
	// is_ref($a) = true
	// refcount($a['foo']) = 1

	var_dump($a);

	extract($a, EXTR_REFS);

	$foo = 'noo';

	var_dump($a);
}

t1();
t2();
t3();
?>
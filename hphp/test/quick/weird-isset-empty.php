<?php

$x = array("a","b","c");
$y = 0;
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

$x = array("a","b","c");
$y = "0";
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

echo "**************************\n";

$x = array(null);
$y = 0;
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

$x = array(null);
$y = "0";
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

echo "**************************\n";

$x = "abc";
$y = 0;
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

$x = "abc";
$y = "0";
var_dump(isset($x[$y]));
var_dump(empty($x[$y]));
var_dump(isset($x->$y));
var_dump(empty($x->$y));
var_dump($x);

echo "**************************\n";

$abc = array('foo' => array('bar' => 'baz'));
$abc2 =& $abc;
var_dump(isset($abc['foo']));
var_dump(empty($abc['foo']));
var_dump(isset($abc->foo));
var_dump(empty($abc->foo));
var_dump(isset($abc['foo']['bar']));
var_dump(empty($abc['foo']['bar']));
var_dump(isset($abc['foo']->bar));
var_dump(empty($abc['foo']->bar));
var_dump(isset($abc->foo['bar']));
var_dump(empty($abc->foo['bar']));
var_dump(isset($abc->foo->bar));
var_dump(empty($abc->foo->bar));

echo "**************************\n";

$abc = new stdclass;
$abc->foo = array('bar' => 'baz');
var_dump(isset($abc->foo));
var_dump(empty($abc->foo));
var_dump(isset($abc->foo['bar']));
var_dump(empty($abc->foo['bar']));
var_dump(isset($abc->foo->bar));
var_dump(empty($abc->foo->bar));
unset($abc);

echo "**************************\n";

$abc = new stdclass;
$abc2 =& $abc; // make $abc a Var
$abc->foo = array('bar' => 'baz');
var_dump(isset($abc->foo));
var_dump(empty($abc->foo));
var_dump(isset($abc->foo['bar']));
var_dump(empty($abc->foo['bar']));
var_dump(isset($abc->foo->bar));
var_dump(empty($abc->foo->bar));
unset($abc);
unset($abc2);

echo "**************************\n";

$abc = array('foo' => new stdclass);
$abc['foo']->bar = 'baz';
var_dump(isset($abc['foo']));
var_dump(empty($abc['foo']));
var_dump(isset($abc->foo));
var_dump(empty($abc->foo));
var_dump(isset($abc['foo']->bar));
var_dump(empty($abc['foo']->bar));
var_dump(isset($abc->foo['bar']));
var_dump(empty($abc->foo['bar']));
var_dump(isset($abc->foo->bar));
var_dump(empty($abc->foo->bar));
unset($abc);

echo "**************************\n";

$abc = array('foo' => new stdclass);
$abc2 =& $abc; // make $abc a Var
$abc['foo']->bar = 'baz';
var_dump(isset($abc['foo']));
var_dump(empty($abc['foo']));
var_dump(isset($abc->foo));
var_dump(empty($abc->foo));
var_dump(isset($abc['foo']->bar));
var_dump(empty($abc['foo']->bar));
var_dump(isset($abc->foo['bar']));
var_dump(empty($abc->foo['bar']));
var_dump(isset($abc->foo->bar));
var_dump(empty($abc->foo->bar));
unset($abc);
unset($abc2);

echo "**************************\n";


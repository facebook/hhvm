<?hh

function foo() {
  $GLOBALS['x']++;
  $GLOBALS['x2'] = 44;
  $GLOBALS['a'][] = "a";
  $GLOBALS['a2'] = varray["a2"];
}

function bar() {
  var_dump($GLOBALS['x']);
  var_dump($GLOBALS['x2']);
  var_dump($GLOBALS['a']);
  var_dump($GLOBALS['a2']);
}

$x = 42;
$a = varray[];
foo();
bar();

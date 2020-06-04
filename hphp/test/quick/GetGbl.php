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

<<__EntryPoint>>
function main() {
  $GLOBALS['x'] = 42;
  $GLOBALS['a'] = varray[];
  foo();
  bar();
}

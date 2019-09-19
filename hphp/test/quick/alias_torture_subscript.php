<?hh

<<__EntryPoint>>
function foo() {

  $GLOBALS['a'] = 123;
  $b = $GLOBALS['GLOBALS'];
  $b['a'] = "of"; // via SetM
  var_dump($GLOBALS['a']);
}

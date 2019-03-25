<?hh // partial

$GLOBALS['foo'] = darray[];
$GLOBALS['foo']['bar'] = 0xba53ba11;
$GLOBALS['bar'] = 0xdeadbeef;
$GLOBALS['baz'] = 0x76543210;

function byRef(&$a) {
  var_dump(__FUNCTION__);
  $a >>= 8;
}

function insideByRef($a) {
  var_dump(__FUNCTION__);
  $z = &$a['bar'];
  $z = 0xfaceb00c;

  $zz = 42;
  $a['baz'] = &$zz;
}


$shadow = $GLOBALS;
var_dump($GLOBALS['foo']);
var_dump($GLOBALS['bar']);
byRef(&$shadow['foo']['bar']);
byRef(&$shadow['bar']);
var_dump($GLOBALS['foo']);
var_dump($GLOBALS['bar']);

$shadow = $GLOBALS;
byRef(&$shadow['foo']['bar']);
byRef(&$shadow['bar']);
var_dump($GLOBALS['foo']);
var_dump($GLOBALS['bar']);

var_dump($GLOBALS['bar']);
var_dump($GLOBALS['baz']);
insideByRef($GLOBALS);
var_dump($GLOBALS['bar']);
var_dump($GLOBALS['baz']);

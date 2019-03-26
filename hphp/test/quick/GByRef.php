<?hh // partial

$GLOBALS['foo'] = darray[];
$GLOBALS['foo']['bar'] = 0xba53ba11;
$GLOBALS['bar'] = 0xdeadbeef;

function byRef(&$a) {
  var_dump(__FUNCTION__);
  $a >>= 8;
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

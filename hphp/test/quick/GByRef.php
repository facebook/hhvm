<?

$GLOBALS['foo'] = array();
$GLOBALS['foo']['bar'] = 0xba53ba11;
$GLOBALS['bar'] = 0xdeadbeef;

function byRef(&$a) {
  $a >>= 16;
}

var_dump($GLOBALS['foo']);
var_dump($GLOBALS['bar']);
byRef($GLOBALS['foo']['bar']);
byRef($GLOBALS['bar']);
var_dump($GLOBALS['foo']);
var_dump($GLOBALS['bar']);

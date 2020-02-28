<?hh


<<__EntryPoint>>
function main_541() {
$GLOBALS['foo'] = 10;
$GLOBALS['bar'] = 
  darray[
      10 => varray[$GLOBALS['foo']],
      20 => varray[$GLOBALS['foo']]];
var_dump($GLOBALS['bar']);
}

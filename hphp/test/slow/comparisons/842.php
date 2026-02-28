<?hh
function foo($a, $b) :mixed{
  $s = (string)$a;
  $t = (string)$b;
  var_dump($s === $t);
}


<<__EntryPoint>>
function main_842() :mixed{
var_dump(HH\Lib\Legacy_FIXME\eq('1.0', '1'));
var_dump('1.0E2' == '10E1');
var_dump('1' === '1');
var_dump('1.0' === '1.0');
var_dump('1' === '1.0');
var_dump('1.0' === '1.00');
var_dump(1.0 === 1.00);
var_dump('1' == '1');
var_dump('1.0' == '1.0');
var_dump(HH\Lib\Legacy_FIXME\eq('1', '1.0'));
var_dump('1.0' == '1.00');
var_dump(1.0 == 1.00);
foo('1.00', '1.0');
foo('1.0', '1.0');
foo('1.', '1.0');
foo('1', '1.0');
}

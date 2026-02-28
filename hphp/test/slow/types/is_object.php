<?hh

<<__EntryPoint>>
function main() :mixed{
  $a = unserialize('O:14:"BogusTestClass":0:{}');
  var_dump(is_object($a));
  var_dump($a is __PHP_Incomplete_Class);

  $a = unserialize('O:8:"IGNOREME":0:{}'); // __PHP_Incomplete_Class
  var_dump(is_object($a));

  $a = 100500;
  var_dump(is_object($a));

  $b = "ololo";
  var_dump(is_object($b));

  // The type not known at compile-time
  $c = new stdClass();
  var_dump(is_object($c));

  // The type known at compile-time
  var_dump(is_object(new stdClass));
}

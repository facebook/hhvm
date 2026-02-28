<?hh

function junk() :mixed{ return 2; }
<<__EntryPoint>>
function bar() :mixed{
  $y = null;
  $x = dict['z' => junk()];
  unset($x['z']);
  try {
    $val = $x['z'];
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
    $val = null;
  }
  var_dump(is_null($val));
  var_dump(is_int($val));
  var_dump(is_array($x));
  var_dump($x);
}

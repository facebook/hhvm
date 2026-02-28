<?hh

function a() :mixed{ return 4; }
function junk() :mixed{ return dict['z' => a()]; }
<<__EntryPoint>>
function bar() :mixed{
  $y = null;
  $x = dict['z' => junk()];
  unset($x['z']['z']);
  $val = $x['z'];
  try { $val1 = $x['z']['z']; }
  catch (Exception $e) {
    echo $e->getMessage()."\n";
    $val1 = null;
  }
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val1));
  var_dump(is_int($val1));
  var_dump(is_array($x));
  var_dump($x);
}

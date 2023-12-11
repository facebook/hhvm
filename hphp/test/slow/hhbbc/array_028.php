<?hh

function junk() :mixed{ return 2; }
function bar() :mixed{
  $x = dict['x' => dict['y' => junk()]];
  $x['x']['y']++;
  $val = $x['x'];
  $val2 = $x['x']['y'];
  var_dump(is_null($val));
  var_dump(is_array($val));
  var_dump(is_null($val2));
  var_dump(is_array($val2));
  var_dump(is_array($x));
  var_dump($x);
}

<<__EntryPoint>>
function main_array_028() :mixed{
bar();
}

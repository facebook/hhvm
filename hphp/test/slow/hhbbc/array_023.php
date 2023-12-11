<?hh

function junk() :mixed{ return 2; }
function bar() :mixed{
  $x = dict['y' => junk(), 'x' => new stdClass()];
  $x['x']->hehe ??= 0;
  $x['x']->hehe += 1;
  $val = $x['x'];
  var_dump(is_null($val));
  var_dump(is_array($x));
}

<<__EntryPoint>>
function main_array_023() :mixed{
bar();
}

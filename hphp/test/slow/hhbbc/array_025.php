<?hh

function junk() :mixed{ return 2; }
function bar() :mixed{
  $x = darray['y' => junk()];
  $x['x'] = 0;
  $x['x'] += 1;
  $val = $x['x'];
  var_dump(is_null($val));
  var_dump(is_array($val));
}

<<__EntryPoint>>
function main_array_025() :mixed{
bar();
}

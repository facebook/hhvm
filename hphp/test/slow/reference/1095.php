<?hh

function foo(inout $perms, inout $t) :mixed{
  $perms = dict['x' => 1];
  $t = $t['x'];
  $t = null;
  return $perms;
}

<<__EntryPoint>>
function main_1095() :mixed{
  $a = null;
  var_dump(foo(inout $a, inout $a));
}

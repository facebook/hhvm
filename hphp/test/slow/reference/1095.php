<?hh

function foo(inout $perms, inout $t) :mixed{
  $perms = dict['x' => 1];
  $t = $t['x'];
  unset($t);
  return $perms;
}

<<__EntryPoint>>
function main_1095() :mixed{
  $a = null;
  try {
    var_dump(foo(inout $a, inout $a));
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

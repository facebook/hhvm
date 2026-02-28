<?hh

function bar() : mixed{
  return 5;
}

<<__EntryPoint>>
function main() {
  $has_val = false;
  apc_fetch('foo', inout $has_val);
  if (!$has_val) apc_add('foo', 1);
  var_dump($has_val);

  $id = $has_val ? '1' : '2';
  require_once(__DIR__."/func-unique-$id.inc");

  var_dump(bar());
  for ($i = 0; $i < 10; $i++) {
    var_dump(outside_bar());
  }
}

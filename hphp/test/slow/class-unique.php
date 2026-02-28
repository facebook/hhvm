<?hh

<<__EntryPoint>>
function main() {
  $has_val = false;
  apc_fetch('foo', inout $has_val);
  if (!$has_val) apc_add('foo', 1);
  var_dump($has_val);

  $id = $has_val ? '1' : '2';
  require_once(__DIR__."/class-unique-$id.inc");
  require_once(__DIR__."/class-unique-bar.inc");

  bar(); bar(); bar(); bar();
}

<?hh

<<__EntryPoint>>
function main() {
  $has_val = false;
  apc_fetch('foo', inout $has_val);
  if (!$has_val) apc_add('foo', 1);
  var_dump($has_val);

  $id = $has_val ? '1' : '2';
  require_once(__DIR__."/unique_enum-$id.inc");

  return var_dump(C::A + C::B + C::C + C::D + C::E + C::F + C::G + C::H + C::I + C::J + C::K + C::L + C::M + C::N + C::O + C::P);
}

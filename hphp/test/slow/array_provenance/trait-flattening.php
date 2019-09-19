<?hh

include dirname(__FILE__) . "/trait-flattening.inc";

class foo {
  use baz;
}

<<__EntryPoint>>
function main() {
  $x = new foo;
  var_dump(HH\get_provenance($x->frob()));
  var_dump(HH\get_provenance($x->fizaz()));
}

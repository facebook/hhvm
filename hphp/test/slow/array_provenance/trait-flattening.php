<?hh

<<__EntryPoint>>
function main() {
  include __DIR__."/trait-flattening.inc";
  include __DIR__."/trait-flattening-class.inc";

  $x = new foo;
  var_dump(HH\get_provenance($x->frob()));
  var_dump(HH\get_provenance($x->fizaz()));
}

<?hh

<<__EntryPoint>>
function main() {
  require "nested/inner.inc";
  require "nested2/inner.inc";
  require "nested/nested3/inner.inc";

  var_dump(main<>);
  (new C)->inner();
  (new C2)->inner();
  (new C3)->inner();
}

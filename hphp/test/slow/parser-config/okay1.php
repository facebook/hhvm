<?hh

<<__EntryPoint>>
function main() {
  require "nested2/inner.inc";

  var_dump(main<>);
  (new C2)->inner();
}

<?hh

<<__EntryPoint>>
function main() {
  require "inner.inc";
  require "nested3/inner.inc";

  var_dump(fun('main'));
  (new C)->inner();
  (new C3)->inner();
}

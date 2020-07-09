<?hh

<<__EntryPoint>>
function main() {
  require "nested/inner.inc";
  require "nested2/inner.inc";
  require "nested/nested3/inner.inc";

  var_dump(fun('main'));
  inner();
  inner2();
  inner3();
}

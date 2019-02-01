<?hh

require "nested/inner.inc";

<<__EntryPoint>>
function main() {
  var_dump(fun('main'));
  inner();
}

<?hh

require "inner.inc";
require "nested3/inner.inc";

<<__EntryPoint>>
function main() {
  var_dump(fun('main'));
  inner();
  inner3();
}

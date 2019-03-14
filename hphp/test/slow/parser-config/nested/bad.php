<?hh

require __DIR__."/../nested2/inner.inc";

<<__EntryPoint>>
function main() {
  var_dump(fun('main'));
  inner2();
}

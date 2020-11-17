<?hh

<<__EntryPoint>>
function main() {
  require __DIR__."/../nested2/inner.inc";

  var_dump(main<>);
  (new C2)->inner();
}

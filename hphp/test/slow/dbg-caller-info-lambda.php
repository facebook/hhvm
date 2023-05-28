<?hh

function dump_info() {
  return hphp_debug_caller_info();
}

class Foo {
  function bar() { return () ==> var_dump(dump_info()); }
  static function sbar() { return () ==> var_dump(dump_info()); }
}

<<__EntryPoint>>
function main() {
  var_dump(dump_info());
  var_dump(() ==> { return dump_info(); }());
  var_dump(() ==> { return dump_info(); }());
  $c1 = (new Foo)->bar();
  $c2 = Foo::sbar();

  $c1();
  $c2();
}

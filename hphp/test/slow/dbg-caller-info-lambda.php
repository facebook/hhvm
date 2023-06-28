<?hh

function dump_info() :mixed{
  return hphp_debug_caller_info();
}

class Foo {
  function bar() :mixed{ return () ==> var_dump(dump_info()); }
  static function sbar() :mixed{ return () ==> var_dump(dump_info()); }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(dump_info());
  var_dump(() ==> { return dump_info(); }());
  var_dump(() ==> { return dump_info(); }());
  $c1 = (new Foo)->bar();
  $c2 = Foo::sbar();

  $c1();
  $c2();
}

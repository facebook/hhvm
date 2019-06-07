<?hh

class C {
  const X = 1;

  static function f() {
    return new C();
  }
}


<<__EntryPoint>>
function main_static_call_access() {
var_dump(C::f()::X);
var_dump(C::{'f' . ''}()::X);
}

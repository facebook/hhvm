<?hh

function a() { echo __METHOD__."\n"; }
function b() { echo __METHOD__."\n"; }
function c() { echo __METHOD__."\n"; }

trait T {
  function f() { echo __METHOD__."\n"; }
  function g() { echo __METHOD__."\n"; }
  function h() { echo __METHOD__."\n"; }

  static function i() { echo __METHOD__."\n"; }
  static function j() { echo __METHOD__."\n"; }
  static function k() { echo __METHOD__."\n"; }
}

class P {
  function x() { echo __METHOD__."\n"; }
  function n() { echo __METHOD__."\n"; }

  static function q() { echo __METHOD__."\n"; }
  static function m() { echo __METHOD__."\n"; }
}

class C extends P {
  use T;

  function x() { echo __METHOD__."\n"; }
  function y() { echo __METHOD__."\n"; }
  function z() { echo __METHOD__."\n"; }

  static function q() { echo __METHOD__."\n"; }
  static function r() { echo __METHOD__."\n"; }
  static function s() { echo __METHOD__."\n"; }
}


<<__EntryPoint>>
function main() {
  a();
  hh\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  b();
  __hhvm_intrinsics\launder_value('c')();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(hh\collect_function_coverage());

  T::i();
  C::s();

  hh\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  C::j(); T::k();
  P::q(); P::m();
  C::q(); C::m();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(hh\collect_function_coverage());

  $a = new C;
  $b = __hhvm_intrinsics\launder_value(new P);

  $a->y();
  $a->h();

  hh\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  $a->x(); $a->z(); $a->n(); $a->f(); $a->g();
  $b->x();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(hh\collect_function_coverage());

  hh\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  a(); b(); c();
  T::i(); T::j(); T::k();
  P::q(); P::m();
  C::q(); C::r(); C::s(); C::m(); C::i(); C::j(); C::k();
  (new P)->x(); (new P)->n();
  (new C)->x(); (new C)->y(); (new C)->z(); (new C)->n();
  (new C)->f(); (new C)->g(); (new C)->h();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(hh\collect_function_coverage());
}

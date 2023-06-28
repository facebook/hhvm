<?hh

function a() :mixed{ echo __METHOD__."\n"; }
function b() :mixed{ echo __METHOD__."\n"; }
function c() :mixed{ echo __METHOD__."\n"; }

trait T {
  function f() :mixed{ echo __METHOD__."\n"; }
  function g() :mixed{ echo __METHOD__."\n"; }
  function h() :mixed{ echo __METHOD__."\n"; }

  static function i() :mixed{ echo __METHOD__."\n"; }
  static function j() :mixed{ echo __METHOD__."\n"; }
  static function k() :mixed{ echo __METHOD__."\n"; }
}

class P {
  function x() :mixed{ echo __METHOD__."\n"; }
  function n() :mixed{ echo __METHOD__."\n"; }

  static function q() :mixed{ echo __METHOD__."\n"; }
  static function m() :mixed{ echo __METHOD__."\n"; }
}

class C extends P {
  use T;

  function x() :mixed{ echo __METHOD__."\n"; }
  function y() :mixed{ echo __METHOD__."\n"; }
  function z() :mixed{ echo __METHOD__."\n"; }

  static function q() :mixed{ echo __METHOD__."\n"; }
  static function r() :mixed{ echo __METHOD__."\n"; }
  static function s() :mixed{ echo __METHOD__."\n"; }
}


<<__EntryPoint>>
function main() :mixed{
  a();
  HH\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  b();
  __hhvm_intrinsics\launder_value('c')();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(HH\collect_function_coverage());

  T::i();
  C::s();

  HH\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  C::j(); T::k();
  P::q(); P::m();
  C::q(); C::m();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(HH\collect_function_coverage());

  $a = new C;
  $b = __hhvm_intrinsics\launder_value(new P);

  $a->y();
  $a->h();

  HH\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  $a->x(); $a->z(); $a->n(); $a->f(); $a->g();
  $b->x();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(HH\collect_function_coverage());

  HH\enable_function_coverage();
  echo ">>>>>> COVERAGE_START\n";
  a(); b(); c();
  T::i(); T::j(); T::k();
  P::q(); P::m();
  C::q(); C::r(); C::s(); C::m(); C::i(); C::j(); C::k();
  (new P)->x(); (new P)->n();
  (new C)->x(); (new C)->y(); (new C)->z(); (new C)->n();
  (new C)->f(); (new C)->g(); (new C)->h();
  echo ">>>>>> COVERAGE_STOP\n";
  var_dump(HH\collect_function_coverage());
}

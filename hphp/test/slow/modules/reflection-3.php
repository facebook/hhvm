<?hh


module A;

internal function f() :mixed{}
function g() :mixed{}

class C {
  internal function f() :mixed{}
  static internal function f_static() :mixed{}

  function g() :mixed{}
  static function g_static() :mixed{}
}

internal class D {}

class E {
  internal int $p0;
  public int $p1;

  static internal int $p2;
  static public int $p3;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionFunction('f')->isInternalToModule()));
  var_dump((new ReflectionFunction('g')->isInternalToModule()));

  var_dump((new ReflectionMethod('C::f')->isInternalToModule()));
  var_dump((new ReflectionMethod('C::f_static')->isInternalToModule()));
  var_dump((new ReflectionMethod('C::g')->isInternalToModule()));
  var_dump((new ReflectionMethod('C::g_static')->isInternalToModule()));

  var_dump((new ReflectionClass('C')->isInternalToModule()));
  var_dump((new ReflectionClass('D')->isInternalToModule()));

  var_dump((new ReflectionProperty('E', 'p0')->isInternalToModule()));
  var_dump((new ReflectionProperty('E', 'p1')->isInternalToModule()));
  var_dump((new ReflectionProperty('E', 'p2')->isInternalToModule()));
  var_dump((new ReflectionProperty('E', 'p3')->isInternalToModule()));
}

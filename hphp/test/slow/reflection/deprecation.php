<?hh

function f() {}
<<__Deprecated>>
function fDep() {}

class C {
  public function f() {}
  <<__Deprecated>>
  public function fDep() {}
  public static function sf() {}
  <<__Deprecated>>
  public static function sfDep() {}
}

function is_deprecated(ReflectionFunctionAbstract $f) {
  $name =
    ($f instanceof ReflectionMethod
     ? ($f->getDeclaringClass()->getName().'::')
     : '') . $f->getName();
  echo $name, ": ", $f->isDeprecated() ? 'deprecated' : 'not deprecated', "\n";
}

function test() {
  is_deprecated(new ReflectionFunction('f'));
  is_deprecated(new ReflectionFunction('fDep'));
  is_deprecated(new ReflectionMethod(C::class, 'f'));
  is_deprecated(new ReflectionMethod(C::class, 'fDep'));
  is_deprecated(new ReflectionMethod(C::class, 'sf'));
  is_deprecated(new ReflectionMethod(C::class, 'sfDep'));
  echo 'Done', "\n";
}

test();

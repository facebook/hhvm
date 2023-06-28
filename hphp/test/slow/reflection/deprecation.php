<?hh

function f() :mixed{}
<<__Deprecated>>
function fDep() :mixed{}

class C {
  public function f() :mixed{}
  <<__Deprecated>>
  public function fDep() :mixed{}
  public static function sf() :mixed{}
  <<__Deprecated>>
  public static function sfDep() :mixed{}
}

function is_deprecated(ReflectionFunctionAbstract $f) :mixed{
  $name =
    ($f is ReflectionMethod
     ? ($f->getDeclaringClass()->getName().'::')
     : '') . $f->getName();
  echo $name, ": ", $f->isDeprecated() ? 'deprecated' : 'not deprecated', "\n";
}

function test() :mixed{
  is_deprecated(new ReflectionFunction('f'));
  is_deprecated(new ReflectionFunction('fDep'));
  is_deprecated(new ReflectionMethod(C::class, 'f'));
  is_deprecated(new ReflectionMethod(C::class, 'fDep'));
  is_deprecated(new ReflectionMethod(C::class, 'sf'));
  is_deprecated(new ReflectionMethod(C::class, 'sfDep'));
  echo 'Done', "\n";
}


<<__EntryPoint>>
function main_deprecation() :mixed{
test();
}

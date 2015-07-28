<?hh // strict

class Base {
  const type T = int;
}

class Child extends Base {
}

interface I {
  const type U = bool;
}

class GrandChild extends Child implements I {
}

$rtc = new ReflectionTypeConstant(GrandChild::class, 'T');
var_dump($rtc->getDeclaringClass()->getName());
var_dump($rtc->getClass()->getName());

$rtc = new ReflectionTypeConstant(GrandChild::class, 'U');
var_dump($rtc->getDeclaringClass()->getName());
var_dump($rtc->getClass()->getName());

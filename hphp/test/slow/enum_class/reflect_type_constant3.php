<?hh

enum class Base : mixed {
  const type T = int;
}

enum class Child : mixed extends Base {
}

enum class I : mixed {
  const type U = bool;
}

enum class GrandChild : mixed extends Child, I {
}


<<__EntryPoint>>
function main_reflect_type_constant3() :mixed{
$rtc = new ReflectionTypeConstant(GrandChild::class, 'T');
var_dump($rtc->getDeclaringClass()->getName());
var_dump($rtc->getClass()->getName());

$rtc = new ReflectionTypeConstant(GrandChild::class, 'U');
var_dump($rtc->getDeclaringClass()->getName());
var_dump($rtc->getClass()->getName());
}

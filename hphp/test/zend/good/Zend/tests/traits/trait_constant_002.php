<?hh

class MyClass {
    static function test() :mixed{
      return __TRAIT__;
    }
}

function someFun() :mixed{
  return __TRAIT__;
}

<<__EntryPoint>> function main(): void {
$t = __TRAIT__;
var_dump($t);
$t = MyClass::test();
var_dump($t);
$t = someFun();
var_dump($t);
}

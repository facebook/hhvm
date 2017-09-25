<?hh // strict
class Test {
  const type T = int;
  public static function z() : void {}
}
function bar()  : void {}

async function foo() : Awaitable<void> {
  $z = fun('bar');
  $y = class_meth('Test', 'z');
  $z = type_structure(Test::class, 'T');
  $z = Vector {1, 2,3};
}


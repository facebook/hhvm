<?hh

interface IFooable {
  const FOO = 3;
  public function f(): classname<T>;
}

abstract class A<<<__Enforceable>> reify T as IFooable> {
  final public function f(): classname<T> {
    return HH\ReifiedGenerics\get_classname<T>();
  }

  final public function g(): class<T> {
    return HH\ReifiedGenerics\get_class_from_type<T>();
  }
}

final class Baz extends A<IFooable> {}

enum class MyEnum: IFooable {
  Baz BAZ = new Baz();
}

<<__EntryPoint>>
function main() : void{
  foreach (MyEnum::getValues() as $value) {
    var_dump($value->f());
    $g = $value->g();
    var_dump($g::FOO);
  }
  echo "done\n";
}

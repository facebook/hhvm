<?hh

interface IMyInterface {
  public function foobar(): string;
}

type Foo = IMyInterface;

class MyClass implements Foo {
  public function foobar(): string {
    return 'classes can implement a type alias that refers to an interface';
  }
}

function blah(): void {
  $x = new MyClass();
  echo $x->foobar();
}

<<__EntryPoint>>
function main_class_implements() :mixed{
  blah();
}

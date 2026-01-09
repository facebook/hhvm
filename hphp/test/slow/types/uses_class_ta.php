<?hh

trait MyTrait {
  public string $foobar = "Classes can use trait type aliases";
}

type Foo = MyTrait;

class MyClass {
  use Foo;
}

function blah(): void {
  $x = new MyClass();
  echo $x->foobar;
}

<<__EntryPoint>>
function main_uses_class_ta() :mixed{
  blah();
}

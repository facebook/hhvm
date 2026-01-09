<?hh

class MyClass {
  public function foobar(): void {
    echo "class type aliases can now be instantiated\n";
  }
}

class MyGenericClass<T> {
  private T $value;

  public function __construct(T $value) {
    $this->value = $value;
  }

  public function getValue(): T {
    return $this->value;
  }
}


class MyReifiedGenericClass<reify T> {
  private T $value;

  public function __construct(T $value) {
    $this->value = $value;
  }

  public function getValue(): T {
    return $this->value;
  }
}


type Foo = MyClass;
type GenericFoo = MyGenericClass;
type ReifiedGenericFoo = MyReifiedGenericClass;

function blah(): void {
  $x = new Foo();
  $x->foobar();
  $y = new GenericFoo(42);
  echo $y->getValue();
  echo "\n";
  $z = new ReifiedGenericFoo<string>("foobar");
  echo $z->getValue();
}

<<__EntryPoint>>
function main_class_instantiation() :mixed{
  blah();
}

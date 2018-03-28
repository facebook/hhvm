<?hh

trait MyTrait {}
class MyClass {
  use MyTrait;
}

type Ttrait = MyTrait;

function is_trait(mixed $x): void {
  if ($x is Ttrait) {
    echo "unreached\n";
  }
}

is_trait(new MyClass());

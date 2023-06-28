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


<<__EntryPoint>>
function main_is_expression_ta_trait() :mixed{
is_trait(new MyClass());
}

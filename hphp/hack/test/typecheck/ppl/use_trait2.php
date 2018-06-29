<?hh // strict

// A non ppl class cannot use a ppl trait

<<__PPL>>
trait MyTrait {
}

class MyClass {
  use MyTrait;
}

<?hh // strict

// A ppl class cannot use a non ppl trait

trait MyTrait {
}

<<__PPL>>
class MyClass {
  use MyTrait;
}

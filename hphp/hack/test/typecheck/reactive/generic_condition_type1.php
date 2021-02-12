<?hh // strict
interface IRxMyParent<T> {

  public function foo(T $a): void;
}

abstract class MyParent {

  public function foo(string $s): void {}
}

// OK
class MyChildRx extends MyParent implements IRxMyParent<string> {}

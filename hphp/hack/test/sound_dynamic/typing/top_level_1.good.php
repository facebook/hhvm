<?hh

class A<T> {}

<<__SoundDynamicCallable>>
function foo(A<int> $a) : void {}

<<__SoundDynamicCallable>>
class C {
  public function bar(A<int> $v) : void {
    foo($v);
  }
}

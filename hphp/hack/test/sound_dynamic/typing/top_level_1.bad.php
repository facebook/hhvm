<?hh

class A<T> {}

function foo(A<int> $a) : void {}

<<__SupportDynamicType>>
class C {
  public function bar(A<int> $v) : void {
    foo($v);
  }
}

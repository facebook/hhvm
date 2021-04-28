<?hh

class A<T> {}

<<__SoundDynamicCallable>>
function foo() : A<int> {
  return new A<int>();
}

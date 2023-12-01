<?hh

class A<T> {}

<<__DynamicallyCallable>>
function foo() : A<int> {
  return new A<int>();
}

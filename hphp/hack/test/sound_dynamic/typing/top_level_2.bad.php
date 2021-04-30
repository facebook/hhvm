<?hh

class A<T> {}

<<__SupportDynamicType>>
function foo() : A<int> {
  return new A<int>();
}

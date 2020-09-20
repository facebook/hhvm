<?hh // strict

class A {}

<<__Rx, __MutableReturn>>
function f1(): A {
  return \HH\Rx\mutable(new A());
}

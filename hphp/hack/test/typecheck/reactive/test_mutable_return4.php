<?hh // strict

class A {}

<<__MutableReturn>>
function f1(): A {
  return new A();
}

<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {}

<<__MutableReturn>>
function f1(): A {
  return new A();
}

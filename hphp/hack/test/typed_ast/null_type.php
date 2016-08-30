<?hh // strict

class MyClass {
}

function bar(?MyClass $arg) : void {
}

function foo() : void {
  $x = null;
  bar($x);
  $x = new MyClass();
  bar($x);
}

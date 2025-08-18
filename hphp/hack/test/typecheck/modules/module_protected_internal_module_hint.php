//// module_M.php
<?hh
new module M {}

//// A.php
<?hh

module M;

internal class A {}

//// B.php
<?hh

module M;

abstract class B {
  // OK
  internal abstract function foo(): A;
  // OK
  protected internal abstract function bar(): A;
}

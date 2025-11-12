///// module_a.php
<?hh

new module a {}

///// module_b.php
<?hh

new module b {}

///// a.php
<?hh

module a;

<<file: __EnableUnstableFeatures('sealed_methods')>>

// TODO: it should be an error to use Sealed with a hidden class name
<<__Sealed(D::class)>>
class C {
  <<__Sealed(D::class)>>
  public function foo():void {}
}

///// b.php
<?hh

module b;

internal class D{}

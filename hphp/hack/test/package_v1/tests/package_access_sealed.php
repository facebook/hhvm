//// modules.php
<?hh
new module x {}    // default package
new module a {}    // pkg1

//// x.php
<?hh
module x;
<<__Sealed(A::class)>>
trait TFoo {

}

//// a.php
<?hh
module a;

class A {
  use TFoo;
}

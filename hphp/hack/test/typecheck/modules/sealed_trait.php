//// modules.php
<?hh
new module foo {}

//// test.php
<?hh
module foo;
internal class Foo {
  use TBar;
}


//// test2.php
<?hh
<<__Sealed(Foo::class)>>
trait TBar {}

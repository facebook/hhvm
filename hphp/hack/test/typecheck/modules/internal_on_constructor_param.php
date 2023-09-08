//// modules.php
<?hh

new module foo {}

//// file1.php
<?hh

module foo;

class Foo {
  public function __construct(
    internal int $x = 5
  ) {}
}
//// file2.php
<?hh

function test(Foo $x): void {
  $z = $x->x; // error
}

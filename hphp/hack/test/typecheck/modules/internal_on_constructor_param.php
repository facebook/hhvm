//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {}

//// file1.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;

class Foo {
  public function __construct(
    internal int $x = 5
  ) {}
}
//// file2.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
function test(Foo $x): void {
  $z = $x->x; // error
}

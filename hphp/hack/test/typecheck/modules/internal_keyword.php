//// file1.php
<?hh
<<file:__EnableUnstableFeatures("modules"), __Module("foo")>>
module foo {}
class Foo {
  internal function bar(): void {}
}


//// file2.php
<?hh
function test(): void {
  $x = new Foo();
  $x->bar();

}

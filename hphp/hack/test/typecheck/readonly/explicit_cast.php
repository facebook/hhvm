//// file1.php
<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {}
class Foo {
  public function __construct(public readonly Bar $bar) {}
}
function foo(): readonly Foo {
  return new Foo(readonly new Bar());
}

//// file2.php
<?hh

// note that this file does not have the file attribute

function breakIt() : void {
  // error, foo returns readonly
  $y = foo();
  // error, bar is a readonly prop
  $z = $y->bar;
}

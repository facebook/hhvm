//// module.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module foo {}

//// decl.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
module newtype Foo = FooInternal; // ok
module newtype FooBad as FooInternal = FooInternal; // error
internal class FooInternal {
  public function foo(): void {}

}
function same_file(Foo $x) : void {
  $x->foo(); // ok
}

//// use.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
function same_module(Foo $x) : void {
  $x->foo(); // ok, it's FooInternal
}


//// use_bad.php
<?hh
function bad(Foo $x): void {
  $x->foo();
}

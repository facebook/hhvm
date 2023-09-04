//// base-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// changed-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// base-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

class Foobar {
  public function foo(): void {}
}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

class Foobar {
  public function foo(): void {}
}

//// base-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

function test(Foobar $x): void {
  $x->foo();
}

//// changed-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

function test(Foobar $x): void {
  $x->foo();
}

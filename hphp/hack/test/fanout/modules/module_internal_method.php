//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}

//// base-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

class Foo {
  public function foo(): void {}
}


//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

class Foo {
  <<__Internal>>
  public function foo(): void {}
}

//// base-foo-use.php
<?hh

function call_foo(Foo $f): void {
  $f->foo();
}

//// changed-foo-use.php
<?hh

function call_foo(Foo $f): void {
  $f->foo();
}

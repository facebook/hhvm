//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B.*
  }
}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B.X.*
  }
}

//// base-b-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module B {}

//// changed-b-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module B {}

//// base-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

public class Foo {
  public function foo(): void {}
}


//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

public class Foo {
  public function foo(): void {}
}

//// base-foo-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module B;

function call_foo(Foo $f): void {
  $f->foo();
}

//// changed-foo-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module B;

function call_foo(Foo $f): void {
  $f->foo();
}

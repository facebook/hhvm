//// base-a-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B.*
  }
}

//// changed-a-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    B.X.*
  }
}

//// base-b-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module B {}

//// changed-b-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module B {}

//// base-foo-defn.php
<?hh

module A;

public class Foo {
  public function foo(): void {}
}


//// changed-foo-defn.php
<?hh

module A;

public class Foo {
  public function foo(): void {}
}

//// base-foo-use.php
<?hh

module B;

function call_foo(Foo $f): void {
  $f->foo();
}

//// changed-foo-use.php
<?hh

module B;

function call_foo(Foo $f): void {
  $f->foo();
}

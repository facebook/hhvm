//// base-a-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
    global
  }
}

//// changed-a-decl.php
<?hh

<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  exports {
  }
}

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

function call_foo(Foo $f): void {
  $f->foo();
}

//// changed-foo-use.php
<?hh

function call_foo(Foo $f): void {
  $f->foo();
}

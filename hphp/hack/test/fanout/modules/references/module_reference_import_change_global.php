//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  imports {
    global
  }
}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
<<file: __EnableUnstableFeatures('module_references')>>

new module A {
  imports {
  }
}

//// base-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

public class Foo {
  public function foo(): void {
    bar();
  }
}


//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

public class Foo {
  public function foo(): void {
    bar();
  }
}

//// base-foo-use.php
<?hh

function bar(): void {
}

//// changed-foo-use.php
<?hh

function bar(): void {
}

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

type Foo = int;

//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
type Foo = int;
//// base-foo-use.php
<?hh

function return_foo(): Foo {
  return 42;
}

//// changed-foo-use.php
<?hh

function return_foo(): Foo {
  return 42;
}

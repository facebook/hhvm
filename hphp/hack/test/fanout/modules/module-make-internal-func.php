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

function foo(): void {}

//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
function foo(): void {}
//// base-foo-use.php
<?hh

function call_foo(): void {
  foo();
}

//// changed-foo-use.php
<?hh

function call_foo(): void {
  foo();
}
//// base-foo-use-in-module.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

function call_foo_in_module(): void {
  foo();
}

//// changed-foo-use-in-module.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

function call_foo_in_module(): void {
  foo();
}

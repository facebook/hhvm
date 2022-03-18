//// decls.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
type Foo = int;

<<__Internal>>
class Crosby {}

//// also-A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A')>>

function foobar<T>(T $t): void where T = Foo {
  $t as Foo;
  foobar($t);
}

<<__Internal>>
function quxx<T as Foo>(T $_): void {}

//// not-A.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('B')>>

function baz<T>(T $t): void where T = Foo {
  $t as Foo;
  baz($t);
}

function bing<T as Foo>(T $_): void {}

function get_crosby<T>(): T where T = Crosby {
  invariant_violation('!!!');
}

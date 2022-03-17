//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}

//// changed-a-decl.php
<?hh

//// base-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
class FooA {}

//// changed-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
class FooA {}
//// base-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
function foobar(): void {}
//// changed-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
function foobar(): void {}

//// base-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
newtype Foo = int;
//// changed-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
newtype Foo = int;

//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}
//// base-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

class FooA {}

//// changed-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
class FooA {}

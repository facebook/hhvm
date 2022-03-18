//// base-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// changed-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

module A {}
module B {}

//// base-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
class Foobar {}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('B')>>

<<__Internal>>
class Foobar {}

//// base-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
final class Bing extends Foobar {}

//// changed-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

<<__Internal>>
final class Bing extends Foobar {}

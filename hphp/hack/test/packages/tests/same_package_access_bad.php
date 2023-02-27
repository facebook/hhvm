//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module a {}    // package pkg1
new module b.b2 {} // package pkg1

//// a.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module a;
internal type AInt = int;

//// b.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module b.b2;
internal type BInt = AInt; // error

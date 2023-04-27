//// modules.php
<?hh
new module a {}  // package pkg1
new module c {}  // package pkg3

//// a.php
<?hh
<<file:__EnableUnstableFeatures('package')>>
module a;
public type AInt = int;

//// c.php
<?hh
<<file:__EnableUnstableFeatures('package')>>
module c;
<<__CrossPackage("pkg1")>>
public function test(AInt $a) : void {}

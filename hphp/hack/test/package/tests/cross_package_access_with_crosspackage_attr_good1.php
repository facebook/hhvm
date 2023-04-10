//// modules.php
<?hh
new module a {}     // package pkg1
new module c {}     // package pkg3 (include pkg2)
new module d {}     // package pkg4

//// a.php
<?hh
module a;
public function f1(): void {}

//// d.php
<?hh
module d;
public function f4(): void {}

//// c.php
<?hh
<<file:__EnableUnstableFeatures('package')>>
module c;
<<__CrossPackage("pkg1", "pkg4")>>
public function test() : void {
  f1();
  f4();
}

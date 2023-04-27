//// modules.php
<?hh
new module a {}     // package pkg1
new module c {}     // package pkg3 (include pkg2)

//// a.php
<?hh
module a;
public function f1(): void {}

//// c.php
<?hh
<<file:__EnableUnstableFeatures('package')>>
module c;
<<__CrossPackage("pkg1")>>
public function test() : void {
  f1();
}

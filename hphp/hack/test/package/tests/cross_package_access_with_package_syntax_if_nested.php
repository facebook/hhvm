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
public function test(): void {
  if (package pkg1) {
    if (package pkg4) {
      // both pkg1 and pkg4 are accessible here
      f1();
      f4();
    }
    // only pkg1 is accessible here
    f1(); // ok
    f4(); // error
  }
}

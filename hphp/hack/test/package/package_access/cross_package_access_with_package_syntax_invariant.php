//// module_a.php
<?hh
new module a {}     // package pkg1
//// module_c.php
<?hh
new module c {}     // package pkg3 (include pkg2)
//// module_d.php
<?hh
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
public function test_err() : void {
   while (true) {
      invariant(package pkg1, "");
      f1(); // ok; pkg1 has been loaded
      f4(); // error; pkg4 has not yet been loaded

      invariant(package pkg4, "");
      // both calls should be ok here
      f1();
      f4();

      break;
   }

   // error; access of pkg1 is outside the scope
   // of invariant(package pkg1, ...) statement
   f1();
}

public function test_ok() : void {
   invariant(package pkg1 && package pkg4, "");
   // both calls should be ok here
   f1();
   f4();
}

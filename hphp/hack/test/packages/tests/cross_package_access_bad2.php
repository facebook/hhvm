//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module a {}     // package pkg1
new module b.b1 {}  // package pkg2 (include pkg1)
new module c {}     // package pkg3 (include pkg2)

//// a.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module a;
public class A {}

//// b.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module b.b1;
public class B1 {}

//// c.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module c;
public class C {}
public function test(): void {
   $b = new B1(); // ok
   $a = new A(); // error
}

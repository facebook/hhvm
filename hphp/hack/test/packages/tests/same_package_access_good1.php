//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module a {}    // package pkg1
new module b.b2 {} // package pkg1

//// a.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module a;
public class A {}
public function test1(): void {
   $b = new B2(); // ok
}

//// b.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module b.b2;
public class B2 {}
public function test2(): void {
   $a = new A(); // ok
}

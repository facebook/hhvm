//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module x {} // package a
new module y.z {} // package b

//// yz.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module y.z;
public class YZ {}
public function test1(): void {
   $x = new X(); // ok
}

//// x.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module x;
public class X {}
public function test2(): void {
   $yz = new YZ();
   // error: member in package b is not visible to package a
}

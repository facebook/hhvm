//// module_a.php
<?hh
new module a {}  // package pkg1
//// module_c.php
<?hh
new module c {}  // package pkg3

//// a.php
<?hh

module a;
public type AInt = int;

//// c.php
<?hh

module c;
<<__CrossPackage("pkg1")>>
public function test(AInt $a) : void {}

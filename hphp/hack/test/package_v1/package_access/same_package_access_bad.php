//// module_a.php
<?hh
new module a {}    // package pkg1
//// module_b.php
<?hh
new module b.b2 {} // package pkg1

//// a.php
<?hh
module a;
internal type AInt = int;

//// b.php
<?hh
module b.b2;
internal type BInt = AInt; // error

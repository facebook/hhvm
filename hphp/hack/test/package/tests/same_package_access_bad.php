//// modules.php
<?hh
new module a {}    // package pkg1
new module b.b2 {} // package pkg1

//// a.php
<?hh
module a;
internal type AInt = int;

//// b.php
<?hh
module b.b2;
internal type BInt = AInt; // error

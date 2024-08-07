//// module_x.php
<?hh
new module x {}    // default package
//// module_y.php
<?hh
new module y {}    // default package

//// x.php
<?hh
module x;
public type XInt = int;

//// y.php
<?hh
module y;
public type YInt = XInt; // ok

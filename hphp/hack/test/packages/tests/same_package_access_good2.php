//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
new module x {}    // default package
new module y {}    // default package

//// x.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module x;
public type XInt = int;

//// y.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

module y;
public type YInt = XInt; // ok

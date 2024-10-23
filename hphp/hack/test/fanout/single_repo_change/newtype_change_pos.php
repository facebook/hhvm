//// base-foo.php
<?hh
newtype Y = int;
//// base-a.php
<?hh
function take_y(Y $_): void {}
//// changed-foo.php
<?hh
// Position change here
newtype Y = int;
//// changed-a.php
<?hh
function take_y(Y $_): void {}

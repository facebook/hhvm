//// base-foo.php
<?hh
type Y = int;
//// base-a.php
<?hh
function take_y(Y $_): void {}
//// changed-foo.php
<?hh
// Position change here
type Y = int;
//// changed-a.php
<?hh
function take_y(Y $_): void {}

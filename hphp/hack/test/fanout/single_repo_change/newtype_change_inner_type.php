//// base-foo.php
<?hh
newtype Y = int;
//// base-a.php
<?hh
function take_y(Y $_): void {}
//// changed-foo.php
<?hh
newtype Y = string;
//// changed-a.php
<?hh
function take_y(Y $_): void {}

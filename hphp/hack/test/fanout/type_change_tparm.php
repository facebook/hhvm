//// base-foo.php
<?hh
type Y<T> = int;
//// base-a.php
<?hh
function take_y<T>(Y<T> $_): void {}
//// changed-foo.php
<?hh
type Y<T as int> = string;
//// changed-a.php
<?hh
function take_y<<<__Explicit>> T>(Y<T> $_): void {}

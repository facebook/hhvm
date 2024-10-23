//// base-a.php
<?hh
type X = int;
//// base-b.php
<?hh
type Y = X;
//// base-c.php
<?hh
function expect_int(int $y): void {}

function f(Y $y): void {
  expect_int($y);
}
//// changed-a.php
<?hh
type X = string;
//// changed-b.php
<?hh
type Y = X;
//// changed-c.php
<?hh
function expect_int(int $y): void {}

function f(Y $y): void {
  expect_int($y);
}

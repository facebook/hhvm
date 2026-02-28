//// file1.php
<?hh

newtype X = int;
newtype Y = string;


// moving down file2.php so line numbers are +10

//// file2.php
<?hh
  
function expect(typename<X> $t): void {}
function f(): void {
  expect(X::class);
  expect(nameof X);
  expect(Y::class);
  expect(nameof Y);
}

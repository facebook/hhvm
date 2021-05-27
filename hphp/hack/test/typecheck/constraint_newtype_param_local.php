//// a.php
<?hh

newtype X<T as arraykey> = T;

function make<T>(T $x): X<T> {
  return $x;
}

//// b.php
<?hh

function foo(): void {
  make(true);
}

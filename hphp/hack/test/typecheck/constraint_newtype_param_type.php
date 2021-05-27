//// a.php
<?hh

newtype X<T as arraykey> = T;
type Y<T> = X<T>;

function make<T>(T $x): Y<T> {
  return $x;
}

//// b.php
<?hh

function foo(): void {
  make(true);
}

<?hh

interface Box<T> {
  public function get(): T;
  public function set(T $x): void;
}

function f(~Box<int> $box): ~int {
  $x = $box->get();
  $box->set($x); // error, only accepts trusted ints
  $box->set(1);
  if ($x !== 1) {
    return $x;
  } else {
    return 1;
  }
}

function g(Box<~int> $box): void {
  $x = $box->get();
  $box->set($x);
  $box->set(1);
}

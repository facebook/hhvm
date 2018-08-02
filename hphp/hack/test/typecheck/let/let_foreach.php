<?hh // experimental

function foo(Iterable<int> $arr): void {
  foreach ($arr as $elem) {
    let x : int = $elem;
  }
  echo x;
}

<?hh // experimental

function foo(array<int> $arr): void {
  let it : Iterable<int> = $arr;
  foreach (it as $elem) {
    let x : int = $elem;
  }
  echo x;
}

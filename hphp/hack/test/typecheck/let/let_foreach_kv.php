<?hh // experimental

function foo(KeyedIterable<int, string> $map): void {
  foreach ($map as $key => $val) {
    let k : int = $key;
    let v : string = $val;
  }
  echo k;
}

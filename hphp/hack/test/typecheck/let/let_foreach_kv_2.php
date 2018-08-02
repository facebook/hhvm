<?hh // experimental

function foo(Map<int, string> $map): void {
  let it : KeyedIterable<int, string> = $map;
  foreach (it as $key => $val) {
    let k : int = $key;
    let v : string = $val;
  }
  echo k;
}

<?hh

function foo(): keyset<int> {
  $b = dict[1 => keyset[1]];
  return $b[null is null ? 1 : 1] ?? keyset[];
}

<<__EntryPoint>>
function main(): void {
  foo();
  echo "PASS";
}

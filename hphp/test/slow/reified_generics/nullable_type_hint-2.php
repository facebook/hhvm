<?hh

function f<reify T>(?T $x): ?T {
  if ($x is T) {
    return $x;
  }
  return null;
}


<<__EntryPoint>>
function main() {
  f<int>(null);
  f<int>(true);
}

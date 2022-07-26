<?hh

<<__Memoize>>
function memo(int $x) {
  var_dump($x);
}

function helper(mixed $f)[ctx $f] {
  memo(1);
  memo(1);
  memo(1);
}

<<__EntryPoint>>
function main() {
  helper(() ==> 1);
}

<?hh

<<__EntryPoint>>
function main() {
  $x = 1;
  function (): int use ($x): int {
    return $x;
  }
}

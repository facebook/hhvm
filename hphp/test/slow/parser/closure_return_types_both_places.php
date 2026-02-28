<?hh

<<__EntryPoint>>
function main() :mixed{
  $x = 1;
  function (): int use ($x): int {
    return $x;
  }
}

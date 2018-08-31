<?hh
function check(mixed $m): bool {
  return $m === null;
}
function crash(vec<int> $a): void {
  if (!$a) check($a);
}

<<__EntryPoint>>
function main_crash() {
$a = vec[];
$a[] = 42;
crash($a);
echo "Done\n";
}

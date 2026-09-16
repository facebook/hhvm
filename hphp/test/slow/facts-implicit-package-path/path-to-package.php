<?hh

function check(string $path, ?string $expected): void {
  $actual = HH\Facts\path_to_package($path);
  echo ($actual === $expected ? 'OK ' : 'FAIL ').$path.' => '.
    ($actual ?? 'null')."\n";
}

<<__EntryPoint>>
function main(): void {
  check('atoms/checkout/member.inc', 'atoms.checkout');
  check('atoms/profile/nested/member.inc', 'atoms.profile');
  check('atoms/unicode/member.inc', 'atoms.unicode');
  check('explicit/member.inc', 'explicit');
  check('explicit/nested/member.inc', 'explicit_nested');
  check('atoms/checkout/override-explicit.inc', 'override');
  check('atoms/missing/member.inc', null);
}

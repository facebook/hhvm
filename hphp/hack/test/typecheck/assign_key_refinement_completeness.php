<?hh

<<__EntryPoint>>
function main(): void {
  $v = 1 === 1 ? vec[0] : null;
  $v[($v as nonnull)[0]] = 42;
  echo $v[0];
}

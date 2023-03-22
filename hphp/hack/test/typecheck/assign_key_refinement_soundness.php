<?hh

<<__EntryPoint>>
function main(): void {
  $v = vec[vec[0]];
  $u = 1 === 2 ? vec[0] : null;
  $v[($u as nonnull)[0]][$u[0]] = 42;
  echo $v[0][0];
}

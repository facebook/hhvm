<?hh

@__EntryPoint
function test(): void {
  $s1 = "@raise @@a";
  $s2 = '@glass @@to';
  // @freedom
  echo "$s1 $s2";
}

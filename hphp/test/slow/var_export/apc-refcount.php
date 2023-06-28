<?hh

function f(string $key, string $input):mixed{
  debug_zval_dump($input);
  apc_add($key, $input);
  $after = __hhvm_intrinsics\apc_fetch_no_check($key);
  debug_zval_dump($after);
}

<<__EntryPoint>>
function main(): void {
  $a = 'a';
  // Static before and after apc
  f('foo1', $a);
  // Counted before and static after apc due promotion to static
  $a[0] = $a;
  f('foo2', $a);
}

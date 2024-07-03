<?hh

<<__Memoize>>
function memo_with_leak_safe_zero_param()[leak_safe]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  $hash = HH\Lib\Str\join($hash, ", ");
  echo "memo_with_leak_safe_zero_param hash:$hash\n";
}

<<__Memoize>>
function memo_with_leak_safe_single_param($a)[leak_safe]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  $hash = HH\Lib\Str\join($hash, ", ");
  echo "memo_with_leak_safe_single_param hash:$hash\n";
}

<<__Memoize>>
function memo_with_pure_zero_param()[]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  $hash = HH\Lib\Str\join($hash, ", ");
  echo "memo_with_pure_zero_param hash:$hash\n";
}

<<__Memoize>>
function memo_with_pure_single_param($a)[]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  $hash = HH\Lib\Str\join($hash, ", ");
  echo "memo_with_pure_single_param hash:$hash\n";
}

<<__Memoize>>
function memo_no_param(): void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  echo "defaults memo_no_param hash:";
  echo HH\Lib\Str\join($hash, ", ");
  echo "\n";
}

<<__Memoize>>
function memo_single_param($a): void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  echo "defaults memo_single_param hash:";
  echo HH\Lib\Str\join($hash, ", ");
  echo "\n";
}

<<__Memoize(#KeyedByIC)>>
function fn_with_zoned() [zoned]: void  {
  return 1;
}

function fn_with_leak_safe() [leak_safe]: void  {
  return 1;
}

<<__Memoize>>
function no_param_with_leaksafe_only()[leak_safe]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  $hash = HH\Lib\Str\join($hash, ", ");
  echo "no_param_with_leaksafe_only hash:$hash";
  echo "\n";
}

<<__Memoize>>
function no_param_with_leaksafe_local_only()[leak_safe_local]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  echo "no_param_with_leaksafe_local_only hash:";
  echo HH\Lib\Str\join($hash, ", ");
  echo "\n";
}

<<__Memoize>>
function no_param_with_leaksafe_shallow_only()[leak_safe_shallow]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  echo "no_param_with_leaksafe_shallow_only hash:";
  echo HH\Lib\Str\join($hash, ", ");
  echo "\n";
}

<<__Memoize>>
function no_param_with_leaksafe_and_defaults()[leak_safe, defaults]: void {
  $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info() ?? 'NULL';
  echo "no_param_with_leaksafe_and_defaults hash:";
  echo HH\Lib\Str\join($hash, ", ");
  echo "\n";
}

<<__EntryPoint>>
function main(): mixed {
  memo_no_param();
  memo_single_param(11);
  memo_with_leak_safe_zero_param();
  memo_with_leak_safe_single_param(12);
  memo_with_pure_zero_param();
  memo_with_pure_single_param(13);
  no_param_with_leaksafe_only();
  no_param_with_leaksafe_and_defaults();
  no_param_with_leaksafe_local_only();
  no_param_with_leaksafe_shallow_only();
}

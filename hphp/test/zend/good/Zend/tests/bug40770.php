<?hh
<<__EntryPoint>> function main(): void {
  ini_set('display_errors', true);
  $mb=148;
  $var = '';
  for ($i = 0; $i <= $mb; $i++) {
    $var .= str_repeat('a', 1024 * 1024);
  }
  // NOTE: Prevent dead-code elimination.
  __hhvm_intrinsics\launder_value($var);
}

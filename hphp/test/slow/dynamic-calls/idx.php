<?hh

<<__EntryPoint>>
function idx_main(): void {
  $key = __hhvm_intrinsics\launder_value('bar');
  var_dump(idx(Map {'foo' => 42}, $key, -1));
}

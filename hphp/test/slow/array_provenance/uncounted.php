<?hh

<<__EntryPoint>>
function main() {
  $a = __hhvm_intrinsics\launder_value(
    dict['crap' => rand(), 'fuck' => rand()]
  );
  apc_store('hello', $a);
  $success = false;
  $b = apc_fetch('hello', inout $success);

  var_dump(HH\get_provenance($a));
  var_dump(HH\get_provenance($b));
}

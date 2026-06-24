<?hh

// Non-string dynamic property names are coerced to their string form for the
// notice (42 -> "42"); arrays fall back to the type without the log throwing.
<<__EntryPoint>>
function main(): void {
  set_error_handler((int $no, string $errstr) ==> {
    echo "notice: ".$errstr."\n";
    return true;
  });

  $o = new stdClass();
  $o->{__hhvm_intrinsics\launder_value(42)} = 'a';     // int    -> 42
  $o->{__hhvm_intrinsics\launder_value(1.5)} = 'b';    // double -> 1.5
  $o->{__hhvm_intrinsics\launder_value(true)} = 'c';   // bool   -> 1
  $o->{__hhvm_intrinsics\launder_value('sk')} = 'd';   // string -> sk
  try {
    $o->{__hhvm_intrinsics\launder_value(vec[1])} = 'e'; // array -> type fallback
    echo "array write ok\n";
  } catch (\Throwable $t) {
    echo "array write threw: ".get_class($t)."\n";
  }
}

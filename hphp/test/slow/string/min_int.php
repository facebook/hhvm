<?hh

<<__EntryPoint>> function main(): void {
  var_dump(0 +  "9223372036854775807");
  var_dump(0 +  "9223372036854775807 ");
  var_dump(0 + "-9223372036854775808");
  var_dump(0 + "-9223372036854775808 ");
}

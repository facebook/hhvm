<?hh

function strstr_wrapped(string $haystack, mixed $needle, bool $before_needle = false)[] : \HH\Runtime\BoolOrString {
  return HH\FIXME\UNSAFE_CAST<mixed, \HH\Runtime\BoolOrString>(strstr($haystack, $needle, $before_needle));
}

function test_bad(): HH\Runtime\BoolOrString {
  return HH\FIXME\UNSAFE_CAST<mixed, \HH\Runtime\BoolOrString>(32);
}
<<__EntryPoint>>
function main():void {
  $email = "name@example.com";
  var_dump(strstr_wrapped($email, "@"));
  var_dump(strstr_wrapped($email, "@", true));
  var_dump(strstr_wrapped($email, "@", false));
  var_dump(strstr_wrapped($email, "!"));
  var_dump(test_bad());
}

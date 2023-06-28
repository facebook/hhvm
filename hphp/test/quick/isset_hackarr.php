<?hh

<<__EntryPoint>>
function main() :mixed{
  $b = dict[123 => 456];
  var_dump(isset($b[123]));
  $b = keyset[123];
  var_dump(isset($b[123]));
}

<?hh

class C {}

function lookup_value(mixed $a, mixed $b): mixed {
  return $b ? $a[get_class($b)] : $b;
}

<<__EntryPoint>>
function main() :mixed{
  $x = dict['C' => 17];
  var_dump(lookup_value($x, new C()));
  var_dump(lookup_value($x, null));
}

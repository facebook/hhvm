<?hh

class Keys {
  const int INT = 123;
}

type MixedKeys = shape('123' => string, Keys::INT => int);

<<__EntryPoint>>
function main(): void {
  $s = __hhvm_intrinsics\launder_value(
    shape('123' => 'string', Keys::INT => 'int'),
  );
  var_dump($s['123'], $s[123]);
  var_dump(array_keys(type_structure(MixedKeys::class)['fields']));
}

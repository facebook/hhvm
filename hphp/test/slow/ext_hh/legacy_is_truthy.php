<?hh

function takes_readonly(readonly Vector<int> $x): bool {
  return HH\legacy_is_truthy($x);
}

<<__EntryPoint>>
function main(): void {
  foreach (
    dict[
      'zero' => 0,
      'one' => 1,
      'string zero' => '0',
      'empty string' => '',
      'string zero dot' => '0.',
      'float zero' => 0.,
      'float small' => 0.00000001,
      'empty vec' => vec[],
      'empty dict' => dict[],
      'empty keyset' => keyset[],
      'non-empty vec' => vec[1, 2, 3],
      'zero vec' => vec[0],
      'resource' => HH\stdin(),
      'null' => null,
      'object' => new stdClass(),
    ]
    as $name => $value
  ) {
    $is_or_is_not = HH\legacy_is_truthy($value) ? 'is' : 'is not';
    echo "$name $is_or_is_not truthy\n";
  }
  takes_readonly(Vector {});
}

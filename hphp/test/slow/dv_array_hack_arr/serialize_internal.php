<?hh

function serialize_internal(mixed $thing): string {
  return __hhvm_intrinsics\serialize_with_format(
    $thing,
    6 /* VariableSerializer::Type::Internal */
  );
}

function serialize_provenance(mixed $thing, bool $keepDVArrays): string {
  return HH\serialize_with_options(
    $thing,
    dict[
      'serializeProvenanceAndLegacy' => true,
      'keepDVArrays' => $keepDVArrays
    ],
  );
}

<<__EntryPoint>>
function main(): void {
  $inputs = vec[
    // empty cases
    vec[],
    HH\array_mark_legacy(vec[]),
    vec[],
    dict[],
    HH\array_mark_legacy(dict[]),
    dict[],
    // non-empty cases
    vec[1, 2, 3],
    HH\array_mark_legacy(vec[1, 2, 3]),
    vec[1, 2, 3],
    dict[0 => 0],
    HH\array_mark_legacy(dict[0 => 0]),
    dict[0 => 0],
  ];
  foreach ($inputs as $input) {
    echo "=========================\n";
    var_dump($input);
    printf("is_marked = %d\n", HH\is_array_marked_legacy($input));
    $serialized = serialize_internal($input);
    printf("%s\n", $serialized);

    invariant(
      $serialized === serialize_provenance($input, true),
      'Expected serialize_provenance to match serialize_internal',
    );
    printf("%s\n", serialize_provenance($input, false));

    $unserialized = unserialize($serialized);
    var_dump($unserialized);
    printf("is_marked = %d\n", HH\is_array_marked_legacy($unserialized));
  }
}

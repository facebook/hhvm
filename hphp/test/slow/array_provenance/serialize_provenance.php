<?hh


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
    varray[],
    HH\array_mark_legacy(varray[]),
    vec[],
    darray[],
    HH\array_mark_legacy(darray[]),
    dict[],
    // non-empty cases
    varray[1, 2, 3],
    HH\array_mark_legacy(varray[1, 2, 3]),
    vec[1, 2, 3],
    darray[0 => 0],
    HH\array_mark_legacy(darray[0 => 0]),
    dict[0 => 0],
  ];
  foreach ($inputs as $input) {
    echo "=========================\n";
    var_dump($input);
    printf("is_marked = %d\n", HH\is_array_marked_legacy($input));
    printf("provenance = %s\n", HH\get_provenance($input));
    $serialized = serialize_provenance($input, true);
    printf("%s\n", $serialized);
    printf("%s\n", serialize_provenance($input, false));

    $unserialized = unserialize($serialized);
    var_dump($unserialized);
    printf("provenance = %s\n", HH\get_provenance($unserialized));
    printf("is_marked = %d\n", HH\is_array_marked_legacy($unserialized));
  }
}

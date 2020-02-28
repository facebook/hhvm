<?hh

function array_count_values_test_cases(
): vec<mixed> {
  return vec[
    tuple(varray[], __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])),
    tuple(
      varray[
        'foo',
        'foo',
        'bar',
        'baz',
        'baz',
        'zar',
      ],
      __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
        'foo' => 2,
        'bar' => 1,
        'baz' => 2,
        'zar' => 1,
      ]),
    ),
    tuple(
      darray[
        'foo' => 1,
        'bar' => 42,
        'baz' => 1,
        'bing' => 42,
        'cro' => 1,
        'sby' => 1,
      ],
      __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
        1 => 4,
        42 => 2,
      ]),
    ),
    tuple(
      Vector {
        0,
        0,
        1,
        0,
        1,
      },
      __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
        0 => 3,
        1 => 2,
      ]),
    ),
    tuple(
      __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[null, 0, false, 0.2]),
      __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[0 => 1]),
    )
  ];
}

<<__EntryPoint>>
function main(): void {
  foreach (array_count_values_test_cases() as $case) {
    list($input, $expected) = $case;
    $output = array_count_values($input);
    invariant($output === $expected, 'Expected output to be equal to expected');
    // We're not changing the output: only asserting that the _input_ can be a
    // Hack Array
    invariant(is_array($output), 'Expected output to be an array');
  }
  // Also test an error case
  array_count_values('foo');
  echo "DONE";
}

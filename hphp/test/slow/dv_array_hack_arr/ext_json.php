<?hh

function decode($input) :mixed{
  // We don't expose this option in userland: we only want to use it internally
  return json_decode($input, true, 512, 1 << 29);
}

// JSON input, is_varray, is_darray
function cases(): vec<(string, bool, bool)> {
  return vec[
    tuple('[]', true, false),
    tuple('{}', false, true),
    tuple('[1, 2, 3]', true, false),
    tuple('{"foo": null}', false, true),
  ];
}

function deep_case() :mixed{
  $res = decode('[{"bar": 0}, [1, 2, 3]]');
  invariant(is_varray($res), 'Expected top-level to be varray');
  invariant(is_darray($res[0]), 'Expected 0th element to be darray');
  invariant(is_varray($res[1]), 'Expected 1st element to be varray');
}

<<__EntryPoint>>
function main() :mixed{
  foreach (cases() as $case) {
    list($input, $is_varray, $is_darray) = $case;
    invariant(
      is_varray(decode($input)) === $is_varray,
      'Expected input to %s varray',
      $is_varray ? 'be' : 'not be',
    );
    invariant(
      is_darray(decode($input)) === $is_darray,
      'Expected input to %s darray',
      $is_varray ? 'be' : 'not be',
    );
  }
  deep_case();
}

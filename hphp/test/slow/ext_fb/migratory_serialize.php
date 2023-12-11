<?hh

function varray_cases() :mixed{
  return vec[
    tuple(vec[1, 2, 3], vec[1, 2, 3]),
    tuple(vec[], vec[]),
  ];
}

function darray_cases() :mixed{
  return vec[
    tuple(dict['foo' => 1, 'bar' => 3], dict['foo' => 1, 'bar' => 3]),
    tuple(dict[0 => 1, 1 => 2], dict[0 => 1, 1 => 2]),
    tuple(dict[], dict[]),
  ];
}

function roundtrip($input) :mixed{
  $ret = null;
  $out = fb_unserialize(
    fb_serialize($input, FB_SERIALIZE_VARRAY_DARRAY),
    inout $ret,
    FB_SERIALIZE_VARRAY_DARRAY,
  );
  invariant($ret, 'Failed to unserialize: %s', var_export($input, true));
  return $out;
}

<<__EntryPoint>>
function main() :mixed{
  foreach (varray_cases() as list($in, $exp)) {
    $out = roundtrip($in);
    invariant(
      is_varray($out),
      'Did not deserailize as varray: %s',
      var_export($out, true),
    );
    invariant(
      $out === $exp,
      'Failed to round trip: %s',
      var_export($in, true),
    );
  }
  foreach (darray_cases() as list($in, $exp)) {
    $out = roundtrip($in);
    invariant(
      is_darray($out),
      'Did not deserailize as darray: %s',
      var_export($out, true),
    );
    invariant(
      $out === $exp,
      'Failed to round trip: %s',
      var_export($in, true),
    );
  }
}

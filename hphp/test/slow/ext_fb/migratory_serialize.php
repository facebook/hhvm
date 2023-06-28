<?hh

function varray_cases() :mixed{
  return vec[
    tuple(varray[1, 2, 3], varray[1, 2, 3]),
    tuple(varray[], varray[]),
  ];
}

function darray_cases() :mixed{
  return vec[
    tuple(darray['foo' => 1, 'bar' => 3], darray['foo' => 1, 'bar' => 3]),
    tuple(darray[0 => 1, 1 => 2], darray[0 => 1, 1 => 2]),
    tuple(darray[], darray[]),
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

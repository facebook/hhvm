<?hh

function track(inout $overall, $name, $result) :mixed{
  if (!array_key_exists($name, $overall)) {
    $overall[$name] = '';
  }
  $overall[$name] .= $result ? '.' : 'X';
}

function trial(inout $overall, $x) :mixed{
  print("\n===========================================================\n");
  print("Input:\n");
  var_dump($x);

  $modes = dict[
    'DEFAULT' => 0,
    'FB_SERIALIZE_VARRAY_DARRAY' => FB_SERIALIZE_VARRAY_DARRAY,
    'FB_SERIALIZE_HACK_ARRAYS' => FB_SERIALIZE_HACK_ARRAYS,
    'FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS' => FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS,
    'FB_SERIALIZE_POST_HACK_ARRAY_MIGRATION' => FB_SERIALIZE_POST_HACK_ARRAY_MIGRATION,
  ];
  foreach ($modes as $name => $mode) {
    print("\n".$name.":\n");
    try {
      $serialized = fb_serialize($x, $mode);
    } catch (Exception $e) {
      var_dump($e->getMessage());
      $serialized = null;
      track(inout $overall, $name, false);
    }
    if ($serialized !== null) {
      $success = null;
      $result = fb_unserialize($serialized, inout $success, $mode);
      if (!$success) throw new Error('Serialization failed!');
      track(inout $overall, $name, $result === $x);
      var_dump($result);
    }
  }
}

function test(inout $overall, $x) :mixed{
  trial(inout $overall, $x);
  if (!($x is keyset<_>)) {
    trial(inout $overall, HH\array_mark_legacy_recursive($x));
  }
}

<<__EntryPoint>>
function main() :mixed{
  $overall = dict[];
  test(inout $overall, vec[]);
  test(inout $overall, dict[]);
  test(inout $overall, keyset[]);
  test(inout $overall, vec[17, 'xy']);
  test(inout $overall, dict[0 => 17, 1 => 'xy']);
  test(inout $overall, dict[17 => 0, 'xy' => 1]);
  test(inout $overall, dict['17' => 0, '34' => 1]);
  test(inout $overall, dict['17' => 0, '34' => 1]);
  test(inout $overall, keyset[17, 'xy']);

  print("\n===========================================================\n");
  foreach ($overall as $name => $result) {
    print($name.":\n");
    print($result."\n");
  }
}

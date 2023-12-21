<?hh

function hexdump(string $s): string {
  $out = '';
  for ($i=0; $i < strlen($s); $i++) {
    $out .= sprintf('%02x', ord($s[$i]));
  }
  return $out;
}

function call_fbcs($value) :mixed{
  return hexdump(fb_compact_serialize($value));
}

function call_fbs($value, $flag) :mixed{
  try {
    return hexdump(fb_serialize($value, $flag));
  } catch (InvalidArgumentException $e) {
    return '<failure>';
  }
}

<<__EntryPoint>>
function main() :mixed{
  $arrays = dict[
    'varray[]' => vec[],
    'darray[]' => dict[],
    'vec[]'    => vec[],
    'dict[]'   => dict[],
    'keyset[]' => keyset[],
  ];

  foreach ($arrays as $label => $array) {
    print("\n============================================================\n");
    print("Testing $label:\n");
    print('gettype:              '); var_dump(gettype($array));
    print('is_php_array:         '); var_dump(HH\is_php_array($array));
    print('is_varray:            '); var_dump(is_varray($array));
    print('is_darray:            '); var_dump(is_darray($array));
    print('is_vec:               '); var_dump($array is vec<_>);
    print('is_vec_fn:            '); var_dump(is_vec($array));
    print('is_dict:              '); var_dump($array is dict<_,_>);
    print('is_dict_fn:           '); var_dump(is_dict($array));
    print('json_encode:          '); var_dump(json_encode($array));
    print('json_hack_arrays:     '); var_dump(json_encode($array, JSON_FB_FORCE_HACK_ARRAYS));
    print('json_php_arrays:      '); var_dump(json_encode($array, JSON_FB_FORCE_PHP_ARRAYS));
    print('serialize:            '); var_dump(serialize($array));
    print('fb_compact_serialize: '); var_dump(call_fbcs($array));

    $flags = vec[
      0,
      FB_SERIALIZE_HACK_ARRAYS,
      FB_SERIALIZE_VARRAY_DARRAY,
      FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS,
    ];
    foreach ($flags as $flag) {
      print("fb_serialize_$flag:       "); var_dump(call_fbs($array, $flag));
    }
  }
}

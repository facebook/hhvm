<?hh

function hexdump(string $s): string {
  $out = '';
  for ($i = 0; $i < strlen($s); $i++) {
    $out .= sprintf('%02x', ord($s[$i]));
  }
  return $out;
}

function to_hack_arrays_recursive($value) :mixed{
  if (HH\is_vec_or_varray($value)) {
    $result = vec[];
    foreach ($value as $v) {
      $result[] = to_hack_arrays_recursive($v);
    }
    return $result;
  } else if (HH\is_dict_or_darray($value)) {
    $result = dict[];
    foreach ($value as $k => $v) {
      $result[$k] = to_hack_arrays_recursive($v);
    }
    return $result;
  }
  return $value;
}

function test($value) :mixed{
  print("\n==============================================================\n");
  $value_str = json_encode($value, JSON_FB_FORCE_HACK_ARRAYS);
  print("test($value_str):\n");

  $hack_arrays = to_hack_arrays_recursive($value);
  $options = FB_COMPACT_SERIALIZE_FORCE_PHP_ARRAYS;
  $label = 'Hack arrays (flag off): ';
  print($label.hexdump(fb_compact_serialize($hack_arrays, 0))."\n");
  $label = 'Hack arrays (flag on):  ';
  print($label.hexdump(fb_compact_serialize($hack_arrays, $options))."\n");

  $mark_arrays = HH\array_mark_legacy_recursive($value);
  $label = 'Legacy-marked arrays:   ';
  print($label.hexdump(fb_compact_serialize($mark_arrays))."\n");
}

<<__EntryPoint>>
function main() :mixed{
  test(vec[]);
  test(vec[vec[]]);
  test(darray(vec[17, 34]));
  test(vec[darray(vec[17, 34])]);
}

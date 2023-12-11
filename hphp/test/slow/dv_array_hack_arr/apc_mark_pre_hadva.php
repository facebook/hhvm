<?hh

function round_trip_in_apc(mixed $val): mixed {
  $err = null;
  apc_store('abc', $val);
  return apc_fetch('abc', inout $err);
}
<<__EntryPoint>>
function main() :mixed{
  $marked_arr = HH\array_mark_legacy(vec[4, 5, 6]);
  $marked_arr = round_trip_in_apc($marked_arr);
  var_dump(HH\is_array_marked_legacy($marked_arr));

  $unmarked_arr = vec[4, 5, 6];
  $unmarked_arr = round_trip_in_apc($unmarked_arr);
  var_dump(HH\is_array_marked_legacy($unmarked_arr));
}


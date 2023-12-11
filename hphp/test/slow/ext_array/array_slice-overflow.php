<?hh

const OFFSET = 0x7FFFFFFFFFFFFFFF;

function make_array() :mixed{
  return array_slice(
    vec[
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ],
    OFFSET,
    OFFSET,
    true
  );
}


<<__EntryPoint>>
function main_array_slice_overflow() :mixed{
var_dump(make_array());
}

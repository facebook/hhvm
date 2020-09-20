<?hh

<<__EntryPoint>>
function main() {
  var_dump(fb_compact_serialize(varray[1, 2, 3]));
  var_dump(fb_compact_serialize(darray[1 => 1, 2 => 2, 3 => 3]));
  var_dump(fb_compact_serialize(vec[1, 2, 3]));
  var_dump(fb_compact_serialize(dict[1 => 1, 2 => 2, 3 => 3]));
  var_dump(fb_compact_serialize(keyset[1, 2, 3]));
}

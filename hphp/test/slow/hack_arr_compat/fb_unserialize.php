<?hh

<<__EntryPoint>>
function main () {
  $result = false;
  var_dump(fb_unserialize(fb_serialize(["1" => 1]), &$result));
  var_dump(fb_unserialize(fb_serialize(dict["1" => 1],
                                       FB_SERIALIZE_HACK_ARRAYS),
                          &$result));
  var_dump(fb_unserialize(fb_serialize(dict["1" => 1],
                                       FB_SERIALIZE_HACK_ARRAYS),
                          &$result, FB_SERIALIZE_HACK_ARRAYS));
  var_dump(fb_unserialize(fb_compact_serialize(dict["1" => 1]),
                          &$result, FB_SERIALIZE_HACK_ARRAYS));
}

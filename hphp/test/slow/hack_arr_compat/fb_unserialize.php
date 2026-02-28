<?hh

<<__EntryPoint>>
function main () :mixed{
  $result = false;
  var_dump(fb_unserialize(fb_serialize(darray(dict["1" => 1])), inout $result));
  var_dump(fb_unserialize(fb_serialize(dict["1" => 1],
                                       FB_SERIALIZE_HACK_ARRAYS),
                          inout $result));
  var_dump(fb_unserialize(fb_serialize(dict["1" => 1],
                                       FB_SERIALIZE_HACK_ARRAYS),
                          inout $result, FB_SERIALIZE_HACK_ARRAYS));
  var_dump(fb_unserialize(fb_compact_serialize(dict["1" => 1]),
                          inout $result, FB_SERIALIZE_HACK_ARRAYS));
}

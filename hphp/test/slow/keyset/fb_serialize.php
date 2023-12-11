<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function serde($v) :mixed{
  $ret = null;
  try {
    $ser = fb_serialize($v, FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
    $unser = fb_unserialize($ser, inout $ret, FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
    var_dump($ret);
    var_dump($unser);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $ser = fb_serialize($v, FB_SERIALIZE_HACK_ARRAYS);
    $unser = fb_unserialize($ser, inout $ret, FB_SERIALIZE_HACK_ARRAYS);
    var_dump($ret);
    var_dump($unser);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $ser = fb_serialize($v);
    $unser = fb_unserialize($ser, inout $ret);
    var_dump($ret);
    var_dump($unser);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_fb_serialize() :mixed{
serde(keyset[]);
serde(keyset[1, 2, 3]);
serde(keyset['a', 'b', 'c']);
serde(vec[1, 2, keyset[1, 2, 3], keyset['a', 'b', 'c'], keyset[]]);
serde(vec[1, 2, keyset[1, 2, 3], keyset['a', 'b', 'c'], keyset[]]);
serde(dict['a' => 1,
       'b' => 2,
       'c' => keyset[1, 2, 3],
       'd' => keyset['a', 'b', 'c'],
       'e' => keyset[]]);
serde(dict['a' => 1,
           'b' => 2,
           'c' => keyset[1, 2, 3],
           'd' => keyset['a', 'b', 'c'],
           'e' => keyset[]]);
}

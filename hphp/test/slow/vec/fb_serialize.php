<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function serde($v) :mixed{
  try {
    $ret = null;
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
serde(vec[]);
serde(vec[1, 2, 3]);
serde(vec['a', 'b', 'c']);
serde(vec[vec[1, 2, 3], vec['a', 'b', 'c'], 100, 'abc']);
serde(vec[dict[1 => 'abc', 2 => 'def', 3 => 'ghi'],
          dict['a' => 1, 'b' => 2, 'c' => 3], 100, 'abc']);
serde(vec[vec[1, 2, 3], vec['a', 'b', 'c'], 100, 'abc']);
serde(vec[dict['1' => 100, 1 => 200]]);
serde(vec[keyset['a', 'b', 'c'], keyset[1, 2, 3]]);
serde(vec[1, 2, vec[1, 2, 3], vec['a', 'b', 'c'], vec[]]);
serde(dict['a' => 1,
       'b' => 2,
       'c' => vec[1, 2, 3],
       'd' => vec['a', 'b', 'c'],
       'e' => vec[]]);
}

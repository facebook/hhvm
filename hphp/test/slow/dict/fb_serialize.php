<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function serde($v) {
  try {
    $ser = fb_serialize($v, FB_SERIALIZE_HACK_ARRAYS);
    $unser = fb_unserialize($ser, &$ret, FB_SERIALIZE_HACK_ARRAYS);
    var_dump($ret);
    var_dump($unser);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $ser = fb_serialize($v);
    $unser = fb_unserialize($ser, $ret);
    var_dump($ret);
    var_dump($unser);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}


<<__EntryPoint>>
function main_fb_serialize() {
serde(dict[]);
serde(dict[1 => 'a', 2 => 'b', 3 => 'c']);
serde(dict['a' => 1, 'b' => 2, 'c' => 3]);
serde(dict['first' => vec[1, 2, 3],
           'second' => vec['a', 'b', 'c'],
           'third' => 100,
           'fourth' => 'abc']);
serde(dict[
        123 => dict[1 => 'abc', 2 => 'def', 3 => 'ghi'],
        456 => dict['a' => 1, 'b' => 2, 'c' => 3],
        789 => 100,
        987 => 'abc']);
serde(dict[
        'a' => [1, 2, 3],
        'b' => ['a', 'b', 'c'],
        'c' => 100,
        'd' => 'abc']);
serde(dict['1' => 100, 1 => 200]);
serde(['a' => 1,
       'b' => 2,
       'c' => dict['a' => 1, 'b' => 2, 'c' => 3],
       'd' => dict[1 => 'a', 2 => 'b', 3 => 'c'],
       'e' => dict[]]);
}

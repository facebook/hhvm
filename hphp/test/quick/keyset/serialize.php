<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function roundtrip($ks) :mixed{
  echo "====================================================\n";
  var_dump($ks);
  $str = serialize($ks);
  var_dump($str);
  $ks2 = unserialize($str);
  var_dump($ks2);
}

function try_serialize($val) :mixed{
  try {
    echo "====================================================\n";
    var_dump($val);
    var_dump(serialize($val));
  } catch (Exception $e) {
    echo "Serialize exception: " . $e->getMessage() . "\n";
  }
}

function try_unserialize($val) :mixed{
  try {
    echo "====================================================\n";
    var_dump($val);
    var_dump(unserialize($val));
  } catch (Exception $e) {
    echo "Unserialize exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>> function main(): void {
  roundtrip(keyset[]);
  roundtrip(keyset[1, 2, 3]);
  roundtrip(keyset['a', 'b', 'c']);
  roundtrip(keyset['a', 1, 'b', 2]);
  roundtrip(keyset[2, 'b', 1, 'a']);
  roundtrip(keyset[123, '123']);

  try_unserialize("k:0:{}");
  try_unserialize("k:3:{i:123;s:3:\"abc\";i:456;}");
  try_unserialize("k:2:{i:123;s:3:\"123\";}");

  // Invalid values
  try_unserialize("k:1:{b:0;}");
  try_unserialize("k:1:{d:1.23;}");
  try_unserialize("k:1:{N;}");

  // "Weak" references
  try_unserialize("k:2:{i:123;r:1;}");
  try_unserialize("a:3:{i:123;s:3:\"abc\";i:456;i:101;i:456;k:2:{r:2;r:3;}}");

  // No references
  try_unserialize("k:1:{R:1;}");
  try_unserialize("a:2:{i:123;s:3:\"abc\";i:456;k:1:{R:2;}}");
  try_unserialize("a:2:{i:123;k:1:{i:731;}i:456;R:3;}");
}

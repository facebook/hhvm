<?hh // strict

function pass_by_ref(&$k) {}

function ref_param($k) {
  try {
    pass_by_ref($k[1]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($k[10]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($k["key"]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    pass_by_ref($k[false]);
  } catch (Exception $e) {
    echo "ref_param exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($k);
}

function elem_ref($k) {
  try {
    $elem = &$k[1];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$k[10];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$k["key"];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  try {
    $elem = &$k[false];
  } catch (Exception $e) {
    echo "elem_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($k);
}

function append_ref($k) {
  $value = "some-value";
  try {
    $k[] = &$value;
  } catch (Exception $e) {
    echo "append_ref exception: \"", $e->getMessage(), "\"\n";
  }
  var_dump($k);
}

/*
function ref_unserialize() {
  $ref_str = "D:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  var_dump(unserialize($ref_str));
}
*/

function main() {
  $k = keyset[0, 1, 2, 3];
  ref_param($k);
  elem_ref($k);
  append_ref($k);
  // ref_unserialize();
}
main();

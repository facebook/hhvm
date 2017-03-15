<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 implements HH\IMemoizeParam {
  public function getInstanceKey() { return "Cls1 memo key"; }
}

class Cls2 {}

function escaped_print($x) {
  if (!is_string($x)) {
    var_dump($x);
    return;
  }
  var_dump(quoted_printable_encode($x));
}

function test() {
  escaped_print(HH\serialize_memoize_param(null));
  escaped_print(HH\serialize_memoize_param(true));
  escaped_print(HH\serialize_memoize_param(false));
  escaped_print(HH\serialize_memoize_param(123));
  escaped_print(HH\serialize_memoize_param(""));
  escaped_print(HH\serialize_memoize_param("hello world"));
  escaped_print(HH\serialize_memoize_param("123"));
  escaped_print(HH\serialize_memoize_param([]));
  escaped_print(HH\serialize_memoize_param(vec[]));
  escaped_print(HH\serialize_memoize_param(dict[]));
  escaped_print(HH\serialize_memoize_param(keyset[]));
  escaped_print(HH\serialize_memoize_param(Vector {}));
  escaped_print(HH\serialize_memoize_param(Set {}));
  escaped_print(HH\serialize_memoize_param(Map {}));

  escaped_print(HH\serialize_memoize_param("~abcdef"));
  escaped_print(HH\serialize_memoize_param(1.234));

  escaped_print(HH\serialize_memoize_param([1, 2, 3]));
  escaped_print(HH\serialize_memoize_param(vec[1, 2, 3]));
  escaped_print(HH\serialize_memoize_param(dict['a' => 1, 'b' => 2, 'c' => 3]));
  escaped_print(HH\serialize_memoize_param(keyset[1, 2, 3]));

  escaped_print(HH\serialize_memoize_param(Vector { 1, 2, 3 }));
  escaped_print(HH\serialize_memoize_param(Set { 1, 2, 3 }));
  escaped_print(HH\serialize_memoize_param(Map { 'a' => 1, 'b' => 2, 'c' => 3 }));
  escaped_print(HH\serialize_memoize_param(Pair { 1, 2 }));

  escaped_print(HH\serialize_memoize_param(new Cls1()));

  try {
    escaped_print(HH\serialize_memoize_param(new Cls2()));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  error_reporting(0);

  try {
    escaped_print(HH\serialize_memoize_param(imagecreate(1, 1)));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

test();

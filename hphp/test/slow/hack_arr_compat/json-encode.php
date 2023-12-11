<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function literals() :mixed{
  echo "================== literals ==================\n";
  json_encode(
    dict[],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    darray(vec[1, 2, 3, 4]),
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    vec[],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    vec[1, 2, 3, 4],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    dict[],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    dict['a' => 1, 'b' => 2, 'c' => 3],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    keyset[],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    keyset['a', 'b', 'c'],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );

  json_encode(
    darray(vec[darray(vec[1, 2]), darray(vec[3, 4])]),
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    vec[vec[1, 2], vec[3, 4]],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
  json_encode(
    dict['a' => dict['c' => 1, 'd' => 2], 'b' => dict['e' => 3, 'f' => 4]],
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );

  json_encode(
    darray(vec[1, vec[2, 3, 4], darray(vec[5, 6, 7]), keyset[8, 9, 10], dict['a' => 11, 'b' => 12]]),
    JSON_FB_WARN_DICTS | JSON_FB_WARN_KEYSETS | JSON_FB_WARN_PHP_ARRAYS
  );
}

function test($name, $options) :mixed{
  echo "==================== $name =========================\n";
  var_dump(json_encode(dict[], $options));
  var_dump(json_encode(darray(vec[1, 2, 3, 4]), $options));
  var_dump(json_encode(vec[], $options));
  var_dump(json_encode(vec[1, 2, 3, 4], $options));
  var_dump(json_encode(dict[], $options));
  var_dump(json_encode(dict['a' => 1, 'b' => 2, 'c' => 3], $options));
  var_dump(json_encode(dict[0 => 'a', 1 => 'b', 2 => 'c'], $options));
  var_dump(json_encode(dict[], $options));
  var_dump(json_encode(dict['a' => 1, 'b' => 2, 'c' => 3], $options));
  var_dump(json_encode(dict[0 => 'a', 1 => 'b', 2 => 'c'], $options));
  var_dump(json_encode(keyset[], $options));
  var_dump(json_encode(keyset['a', 'b', 'c'], $options));

  var_dump(json_encode(darray(vec[darray(vec[1, 2]), darray(vec[3, 4])]), $options));
  var_dump(json_encode(vec[vec[1, 2], vec[3, 4]], $options));
  var_dump(json_encode(
    dict['a' => dict['c' => 1, 'd' => 2], 'b' => dict['e' => 3, 'f' => 4]],
    $options
  ));

  var_dump(json_encode(
    darray(vec[1, vec[2, 3, 4], darray(vec[5, 6, 7]), keyset[8, 9, 10], dict['a' => 11, 'b' => 12]]),
    $options
  ));

  $obj = new stdClass();
  $obj->a = darray(vec[1, 2, 3]);
  $obj->b = vec[1, 2, 3];
  $obj->c = dict['a' => 1, 'b' => 2];
  $obj->d = keyset['a', 'b', 'c'];
  var_dump(json_encode($obj, $options));
  try {
    var_dump(json_encode(new LateInitClass(), $options));
  } catch (Exception $ex) {
    var_dump($ex->getMessage());
  }
}

class LateInitClass {
  <<__LateInit>> public string $prop;
}

<<__EntryPoint>>
function main_json_encode() :mixed{
literals();

test("normal", 0);
test("force PHP", JSON_FB_FORCE_PHP_ARRAYS);
test("force Hack", JSON_FB_FORCE_HACK_ARRAYS);
test("dict warn", JSON_FB_WARN_DICTS);
test("keyset warn", JSON_FB_WARN_KEYSETS);
test("PHP warn", JSON_FB_WARN_PHP_ARRAYS);
test("empty darrays", JSON_FB_WARN_EMPTY_DARRAYS);
test("vec-like darrays", JSON_FB_WARN_VEC_LIKE_DARRAYS);
test("dict-like darrays", JSON_FB_WARN_DICT_LIKE_DARRAYS);
test("all darrays",
  JSON_FB_WARN_EMPTY_DARRAYS |
  JSON_FB_WARN_VEC_LIKE_DARRAYS |
  JSON_FB_WARN_DICT_LIKE_DARRAYS
);
test("warn both", JSON_FB_WARN_DICTS | JSON_FB_WARN_PHP_ARRAYS);
test("force + dict warn", JSON_FB_WARN_DICTS | JSON_FB_FORCE_PHP_ARRAYS);
test("force + PHP warn", JSON_FB_WARN_PHP_ARRAYS | JSON_FB_FORCE_PHP_ARRAYS);
test(
  "everything",
  JSON_FB_WARN_PHP_ARRAYS | JSON_FB_WARN_DICTS | JSON_FB_FORCE_PHP_ARRAYS
);
test("ignore lateinit", JSON_FB_IGNORE_LATEINIT);
echo "==================== no repeated warnings =========================\n";
var_dump(json_encode(vec[
  dict[], dict[],
  dict[0 => 'a'], dict[0 => 'a'],
  dict[1 => 'a'], dict[1 => 'a'],
  dict[0 => 'a'], dict[0 => 'a'],
], JSON_FB_WARN_EMPTY_DARRAYS |
  JSON_FB_WARN_VEC_LIKE_DARRAYS |
  JSON_FB_WARN_DICT_LIKE_DARRAYS |
  JSON_FB_WARN_PHP_ARRAYS |
  JSON_FB_WARN_DICTS |
  JSON_FB_WARN_KEYSETS
));
echo "=============== force Hack arrays ignores structure ===============\n";
$examples = vec[
  vec[],
  dict[],
  vec[],
  dict[],
  keyset[],
  vec[17, 34],
  dict[0 => 17, 1 => 34],
  vec[17, 34],
  dict[0 => 17, 1 => 34],
  keyset[17, 34],
  dict['a' => 17, 'b' => 34],
  dict['a' => 17, 'b' => 34],
  keyset['a', 'b'],
];
foreach ($examples as $example) {
  var_dump(json_encode($example, JSON_FB_FORCE_HACK_ARRAYS));
}
}

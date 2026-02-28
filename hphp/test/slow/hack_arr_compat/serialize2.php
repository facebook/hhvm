<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function literals() :mixed{
  echo "================== literals ==================\n";
  HH\serialize_with_options(
    dict[],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    darray(vec[1, 2, 3, 4]),
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    vec[],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    vec[1, 2, 3, 4],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    dict[],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    dict['a' => 1, 'b' => 2, 'c' => 3],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    keyset[],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    keyset['a', 'b', 'c'],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );

  HH\serialize_with_options(
    darray(vec[darray(vec[1, 2]), darray(vec[3, 4])]),
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    vec[vec[1, 2], vec[3, 4]],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
  HH\serialize_with_options(
    dict['a' => dict['c' => 1, 'd' => 2], 'b' => dict['e' => 3, 'f' => 4]],
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );

  HH\serialize_with_options(
    darray(vec[1, vec[2, 3, 4], darray(vec[5, 6, 7]), keyset[8, 9, 10], dict['a' => 11, 'b' => 12]]),
    dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]
  );
}

function test($name, $options) :mixed{
  echo "==================== $name =========================\n";
  var_dump(HH\serialize_with_options(dict[], $options));
  var_dump(HH\serialize_with_options(darray(vec[1, 2, 3, 4]), $options));
  var_dump(HH\serialize_with_options(vec[], $options));
  var_dump(HH\serialize_with_options(vec[1, 2, 3, 4], $options));
  var_dump(HH\serialize_with_options(dict[], $options));
  var_dump(HH\serialize_with_options(dict['a' => 1, 'b' => 2, 'c' => 3], $options));
  var_dump(HH\serialize_with_options(keyset[], $options));
  var_dump(HH\serialize_with_options(keyset['a', 'b', 'c'], $options));

  var_dump(HH\serialize_with_options(darray(vec[darray(vec[1, 2]), darray(vec[3, 4])]), $options));
  var_dump(HH\serialize_with_options(vec[vec[1, 2], vec[3, 4]], $options));
  var_dump(HH\serialize_with_options(
    dict['a' => dict['c' => 1, 'd' => 2], 'b' => dict['e' => 3, 'f' => 4]],
    $options
  ));

  var_dump(HH\serialize_with_options(
    darray(vec[1, vec[2, 3, 4], darray(vec[5, 6, 7]), keyset[8, 9, 10], dict['a' => 11, 'b' => 12]]),
    $options
  ));

  $obj = new stdClass();
  $obj->a = darray(vec[1, 2, 3]);
  $obj->b = vec[1, 2, 3];
  $obj->c = dict['a' => 1, 'b' => 2];
  $obj->d = keyset['a', 'b', 'c'];
  var_dump(HH\serialize_with_options($obj, $options));
  try {
    var_dump(HH\serialize_with_options(new LateInitClass(), $options));
  } catch (Exception $ex) {
    var_dump($ex->getMessage());
  }
}

class LateInitClass {
  <<__LateInit>> public string $prop;
}

<<__EntryPoint>>
function main_serialize2() :mixed{
literals();

test("normal", dict[]);
test("force PHP", dict["forcePHPArrays" => true]);
test("hack warn", dict["warnOnHackArrays" => true]);
test("PHP warn", dict["warnOnPHPArrays" => true]);
test("warn both", dict["warnOnHackArrays" => true, "warnOnPHPArrays" => true]);
test("force + hack warn", dict["forcePHPArrays" => true, "warnOnHackArrays" => true]);
test("force + PHP warn", dict["forcePHPArrays" => true, "warnOnPHPArrays" => true]);
test(
  "everything",
  dict[
    "forcePHPArrays" => true,
    "warnOnHackArrays" => true,
    "warnOnPHPArrays" => true
  ]
);
test("ignore lateinit", dict["ignoreLateInit" => true]);
}

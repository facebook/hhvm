<?hh

function report($msg, $obj1, $obj2) {
  echo "$msg\nShould be\n";
  var_dump($obj1);
  echo "But found\n";
  var_dump($obj2);
  echo "\n";
}

// Works only when objects consist of
// hack collections and primitive types.
function is_equal($obj1, $obj2) {
  if ($obj1 === null && $obj2 === null) {
    return true;
  }
  $type1 = get_class($obj1);
  $type2 = get_class($obj2);
  if ($type1 !== $type2) {
    report("Incorrect type", $type1, $type2);
    return false;
  }
  if ($type1 !== "HH\Vector" && $type1 !== "HH\Map") {
    return $obj1 === $obj2;
  }
  $n1 = count($obj1);
  $n2 = count($obj2);
  if ($n1 !== $n2) {
    report("Incorrect size", $n1, $n2);
    return false;
  }
  if ($type1 === "HH\Vector") {
    for ($i = 0; $i < $n1; ++$i) {
      if (!is_equal($obj1[$i], $obj2[$i])) {
        report("Incorrect value at [$i]", $obj1[$i], $obj2[$i]);
        return false;
      }
    }
  } else {
    $keys1 = $obj1->toKeysArray();
    $keys2 = $obj2->toKeysArray();
    if ($type1 === "HH\Map") {
      sort($keys1);
      sort($keys2);
    }
    if ($keys1 !== $keys2) {
      report("Incorrect keys", $keys1, $keys2);
      return false;
    }
    foreach ($keys1 as $key) {
      if (!is_equal($obj1[$key], $obj2[$key])) {
        report("Incorrect value at ['$key']", $obj1[$key], $obj2[$key]);
        return false;
      }
    }
  }
  return true;
}

function main() {
  $tests = array(
    // Scalar types tests from HHJsonDecodeTest.php
    array(
      'input' => 'null',
      'options' => 0,
      'expected' => null,
    ),
    array(
      'input' => '0',
      'options' => JSON_FB_LOOSE,
      'expected' => 0,
    ),
    array(
      'input' => '"0"',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => '0',
    ),
    array(
      'input' => 'true',
      'options' => JSON_FB_LOOSE,
      'expected' => true,
    ),
    // Parse Errors tests from HHJsonDecodeTest.php
    array(
      'input' => '',
      'options' => JSON_FB_LOOSE,
      'expected' => null,
    ),
    array(
      'input' => '{[}',
      'options' => JSON_FB_LOOSE,
      'expected' => null,
    ),
    array(
      'input' => '[:(]',
      'options' => JSON_FB_LOOSE,
      'expected' => null,
    ),
    array(
      'input' => 'xxx',
      'options' => JSON_FB_LOOSE,
      'expected' => null,
    ),
    array(
      'input' => 'a {',
      'options' => JSON_FB_LOOSE,
      'expected' => null,
    ),
    // Maps checks from HHJsonDecodeTest.php
    array(
      'input' => '{"0": 1, "a": "b"}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {'0' => 1, 'a' => 'b'},
    ),
    // Vectors/Maps - some basic tests
    array(
      'input' => '[1,2]',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Vector {1, 2},
    ),
    array(
      'input' => '{}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {},
    ),
    array(
      'input' => '{"{" : "}"}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {'{' => '}'},
    ),
    array(
      'input' => '{"0": 1, "a": "b", "[]": null, "#": false}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {'a' => 'b', '0' => 1, '[]' => null, '#' => false},
    ),
    // Collections tests from HHJsonDecodeTest.php
    array(
      'input' => '[]',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Vector {},
    ),
    array(
      'input' => '[null, 0, "0", true]',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Vector {null, 0, '0', true},
    ),
    // LooseModeCollections from HHJsonDecodeTest.phpi
    // Single-quoted strings
    array(
      'input' => '[\'value\']',
      'options' => JSON_FB_LOOSE | JSON_FB_COLLECTIONS,
      'expected' => Vector {'value'},
    ),
    // Unquoted keys
    array(
      'input' => '{key: "value"}',
      'options' => JSON_FB_LOOSE | JSON_FB_COLLECTIONS,
      'expected' => Map {'key' => 'value'},
    ),
    // Boolean keys
    array(
      'input' => '{true: "value"}',
      'options' => JSON_FB_LOOSE | JSON_FB_COLLECTIONS,
      'expected' => Map {'true' => 'value'},
    ),
    // Null keys
    array(
      'input' => '{null: "value"}',
      'options' => JSON_FB_LOOSE | JSON_FB_COLLECTIONS,
      'expected' => Map {'null' => 'value'},
    ),
    // Nested collections
    array(
      'input' =>  '[2,"4",{0:[]}]',
      'options' => JSON_FB_LOOSE | JSON_FB_COLLECTIONS,
      'expected' => Vector {2, '4', Map {'0' => Vector {}}},
    ),
    array(
      'input' => '{"vec": [], "map": {}}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {'vec' => Vector {}, 'map' => Map {}},
    ),
    array(
      'input' => '{"vec" : [{"z" : []}], "map" : {"a" : {"]" : "["}}}',
      'options' => JSON_FB_COLLECTIONS,
      'expected' => Map {
        'vec' => Vector {
          Map {
            'z' => Vector {
            },
          },
        },
        'map' => Map {
          'a' => Map {
            ']' => '[',
          },
        },
      },
    ),
  );
  foreach ($tests as $test) {
    $output = json_decode($test['input'], true, 512, $test['options']);
    if (!is_equal($output, $test['expected'])) {
      report("", $output, $test['expected']);
      break;
    }
  }
  echo "Done\n";
}

main();


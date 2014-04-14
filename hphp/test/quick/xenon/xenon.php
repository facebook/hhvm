<?hh

// Test showing async stacks in Xenon.

async function gen1($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 1;
}

async function gen2($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  $x = gen1($a)->join();
  return $x;
}

async function genBar($a) {
  await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
  return $a + 2;
}

async function genFoo($a) {
  $a++;
  list($x, $y) = await GenArrayWaitHandle::Create(
    array(
      genBar($a),
      genBar($a + 1),
      gen2($a + 2)
    )
  );
  return $x + $y;
}

function idx($arr, $idx, $def = null) {
  return isset($arr[$idx]) ? $arr[$idx] : $def;
}

function main($a) {
  $result = genFoo($a)->join();
}

main(42);

// get the Xenon data then verify that there are no unknown functions
// and that all of the functions in this file are in the stack
$stacks = xenon_get_data();
$functionList = array( "main", "", "WaitHandle->join", "strcasecmp");
$requiredFunctions = array("main" => 1);

$asyncList = array("gen1", "gen2", "genBar", "genFoo", "",
  "<gen-array>", "<prep>");
$requiredAsync = array(
  "gen1" => 1,
  "gen2" => 1,
  "genBar" => 1,
  "genFoo" => 1
);

echo "Verifying PHP Stack\n";
foreach ($stacks as $k => $v) {
  if (is_numeric($k)) {
    $frame = $v["phpStack"];
    if ($frame) {
      foreach ($frame as $f) {
        $function = idx($f, "function", "");
        $file = idx($f, "file", "");
        $line = idx($f, "line", 0);
        if (!in_array($function, $functionList)
            && strpos($function, "Exception") != 0) {
          echo "Unknown function:  " . $function . " " . $file . " "
            . $line . "\n";
        } else {
          $foundFunction = idx($requiredFunctions, $function, "");
          if ($foundFunction) {
            unset($requiredFunctions[$function]);
          }
        } //if in_array
      } // foreach
    } // if ($frame)
  } // if is_numeric
}
if ($requiredFunctions) {
  echo "Functions missing from stacks:  ";
  var_dump($requiredFunctions);
}

echo "Verifying Async Stack\n";
foreach ($stacks as $k => $v) {
  if (is_numeric($k)) {
    $frame = $v["asyncStack"];
    if ($frame) {
      foreach ($frame as $f) {
        $function = idx($f, "function", "");
        $file = idx($f, "file", "");
        $line = idx($f, "line", 0);
        if (!in_array($function, $asyncList)
            && strpos($function, "Exception") != 0) {
          echo "Unknown async function:  " . $function . " " . $file . " "
            . $line . "\n";
        } else {
          $foundFunction = idx($requiredAsync, $function, "");
          if ($foundFunction) {
            unset($requiredAsync[$function]);
          }
        } //if in_array
      } // foreach
    } // if ($frame)
  } // if is_numeric
}
if ($requiredAsync) {
  echo "Functions missing from stacks:  ";
  var_dump($requiredAsync);
}

echo "Finished verfying stacks\n";

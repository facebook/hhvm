<?hh

function main($input) {
  $dynamicArr = array();
  if ($input) {
    $dynamicArr[] = $input;
    $dynamicArr[] = 'sup';
    $dynamicArr[] = 'suup';
    $dynamicArr[] = 'suuup';
    $dynamicArr[] = 'suuuup';
    $dynamicArr[] = 'suuuuup';
  }
}

function harness($times) {
  for ($i = 0; $i < $times; $i++) {
    main(88 % 2 == 0);
  }
}

function test() {
  echo "Creating about 400 PackedVectors via Appending\n";
  harness(200);
  $dump = hphp_array_tracer_dump();
  $numPacked = $dump["PackedVector"];
  $numPackedAppend = $dump["Array Func Calls For PackedKind"]["Append"];
  $numEmptyAppend = $dump["Array Func Calls For EmptyKind"]["Append"];

  echo "Creating another 400 PackedVectors\n";
  harness(200);
  $dump = hphp_array_tracer_dump();
  $newNumPacked = $dump["PackedVector"];
  $newNumPackedAppend = $dump["Array Func Calls For PackedKind"]["Append"];
  $newNumEmptyAppend = $dump["Array Func Calls For EmptyKind"]["Append"];

  echo "Verifying we created an additional 400 PackedVectors\n";
  var_dump($newNumPacked - $numPacked);
  echo "Verifying we performed an additional (200*5) = 1000 Appends "
     . "on Packed\n";
  var_dump($newNumPackedAppend - $numPackedAppend);
  echo "Verifying we performed an additional (200*1) = 200 Appends on Empty\n";
  var_dump($newNumEmptyAppend - $numEmptyAppend);
}

test();

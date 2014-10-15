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
  echo "Creating about 400 PackedVectors\n";
  harness(200);
  $dump = hphp_array_tracer_dump();
  $numPacked = $dump["PackedVector"];

  echo "Creating another 400 PackedVectors\n";
  harness(200);
  $dump = hphp_array_tracer_dump();
  $newNumPacked = $dump["PackedVector"];
  echo "Verifying we created an additional 400 PackedVectors\n";
  var_dump($newNumPacked - $numPacked);
}

test();

<?hh

function main($size) {
  $arr1 = array();
  $arr2 = array();
  for ($i = 0; $i < $size; $i++) {
    if ($i % 2 == 0) {
      $arr1['plus 1 ' . $i] = $i;
      $arr2['plus 1 ' . $i] = $i + 1;
    } else {
      $arr1[] = $i;
      $arr2[] = $i;
    }
  }
  return array_merge($arr1, $arr2);
}

function harness($times) {
  for ($i = 0; $i < $times; $i++) {
    main(10);
  }
}

function test() {
  echo "Creating a bunch of MixedMaps\n";
  harness(100);
  $dump = hphp_array_tracer_dump();
  $numMixedMap = $dump['MixedMap'];

  echo "Creating a some more MixedMaps\n";
  harness(100);

  echo "Checking we accounted for those MixedMaps\n";
  $dump = hphp_array_tracer_dump();
  $newNumMixedMap = $dump['MixedMap'];
  echo "Num new MixedMaps we saw: ";
  var_dump($newNumMixedMap - $numMixedMap);
}

test();

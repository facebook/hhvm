<?hh

function test($s) {
  $det = new EncodingDetector();
  $det->setText($s);

  $matches = $det->detectAll();
  unset($det);

  foreach ($matches as $match) {
    $s .= "::" . $match->getEncoding();
  }
  return $s;
}

function main() {
  for ($i = 0; $i < 10; $i++) {
    var_dump(test("Hello: $i"));
  }
}


<<__EntryPoint>>
function main_detect_crash() {
main();
}

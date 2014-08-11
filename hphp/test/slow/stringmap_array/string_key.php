<?hh

function main() {
  $a = msarray();
  $a['string keys are fine'] = "no warning";
    $a['mixed values are cool too'] = 10;
}

main();

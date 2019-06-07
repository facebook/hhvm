<?hh

function wat(string $x) {
  return 12;
}

function main() {
  try {
    wat(12);
  }
 catch (Exception $x) {
 echo "ok\n";
 }
}


<<__EntryPoint>>
function main_failing_param() {
main();
}

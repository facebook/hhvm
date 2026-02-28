<?hh

function wat(string $x) :mixed{
  return 12;
}

function main() :mixed{
  try {
    wat(12);
  }
 catch (Exception $x) {
 echo "ok\n";
 }
}


<<__EntryPoint>>
function main_failing_param() :mixed{
main();
}

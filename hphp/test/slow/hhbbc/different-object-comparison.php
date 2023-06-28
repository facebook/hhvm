<?hh

function blah() :mixed{
  if (__hhvm_intrinsics\launder_value(false)) return false;
  $d1 = new DateTime("2021-09-24");
  $d2 = new DateTimeImmutable("2021-09-25");
  return $d1 < $d2;
}

<<__EntryPoint>>
function main() :mixed{
  if (blah()) {
    echo "Returned true\n";
  } else {
    echo "Return false\n";
  }
}

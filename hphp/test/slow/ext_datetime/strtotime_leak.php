<?hh

function test() :mixed{
  for ($i = 0; $i < 10000; $i++) {
    strtotime("10 September 2000");
    strtotime("10 September 2000 UTC");
    strtotime("null");
  }
}

function main($report) :mixed{
  $a = memory_get_usage(true);
  test();
  $b = memory_get_usage(true);
  test();
  $c = memory_get_usage(true);
  $v1 = $b - $a;
  $v2 = ($c - $b) * 2;
  if ($report) {
    if ($v2 <= $v1) {
      echo "Ok\n";
    } else {
      echo "strtotime is leaking: $a, $b, $c:  $v1 $v2\n";
    }
  }
}
<<__EntryPoint>>
function main_entry(): void {

  error_reporting(0);

  var_dump(strtotime("10 September 2000 UTC"));
  var_dump(strtotime("null"));

  main(false);
  main(false);
  main(true);
}

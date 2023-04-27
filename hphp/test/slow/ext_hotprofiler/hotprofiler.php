<?hh

function test_level_2() {
  $sum = 0;
  for ($i = 0; $i < 10000; $i++) $sum += $i;
  return $sum;
}

function test_level_1() {
  test_level_2();
  test_level_2();
}

<<__EntryPoint>> function main(): void {
  hotprofiler_enable(2);
  test_level_1();
  $res = hotprofiler_disable();
  $call_count = $res['test_level_1==>test_level_2']['ct'];
  var_dump($call_count);
}

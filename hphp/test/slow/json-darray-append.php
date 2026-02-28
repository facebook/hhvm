<?hh

<<__EntryPoint>>
function main() :mixed{
  $s = __hhvm_intrinsics\launder_value('[{}]');
  $val = json_decode($s, true, 512, JSON_FB_DARRAYS);
  var_dump($val);
  $val[] = 99;
  var_dump($val);
}

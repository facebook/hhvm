<?hh

<<__EntryPoint>>
function main() {
  $v = vec[];
  for ($i = 0; $i < 20; $i++) {
    $v[] = (string)rand();
  }
  var_dump(HH\Lib\Str\format("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", $v[0], $v[1], $v[2], $v[3], $v[4], $v[5], $v[6], $v[7], $v[8], $v[9], $v[10], $v[11], $v[12], $v[13], $v[14], $v[15], $v[16], $v[17], $v[18], $v[19]));
  var_dump($v[0] . $v[1] . $v[2] . $v[3] . $v[4] . $v[5] . $v[6] . $v[7] . $v[8] . $v[9] . $v[10] . $v[11] . $v[12] . $v[13] . $v[14] . $v[15] . $v[16] . $v[17] . $v[18] . $v[19]);
}

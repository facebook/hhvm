<?hh

function main($str) :mixed{
  $arr = vec[];
  for ($i = 0; $i < 3; ++$i) {
    $str[2] = (string)$i;
    $arr[] = $str;
  }
  var_dump($arr);
}
<<__EntryPoint>> function main_entry(): void {
$a = dict['hello there' => 0];
foreach ($a as $key => $value) {
  main($key);
}
}

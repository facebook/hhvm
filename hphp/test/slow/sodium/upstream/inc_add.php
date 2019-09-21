<?hh
<<__EntryPoint>> function main(): void {
$notStr = 123;
try {
  sodium_increment(inout $notStr);
} catch (SodiumException $e) {
  echo $e->getMessage(), "\n";
}

$str = "abc";
$str2 = $str;
sodium_increment(inout $str);
var_dump($str, $str2);

$str = "ab" . ($x = "c");
$str2 = $str;
sodium_increment(inout $str);
var_dump($str, $str2);

$addStr = "\2\0\0";

$notStr = 123;
try {
  sodium_add(inout $notStr, $addStr);
} catch (SodiumException $e) {
  echo $e->getMessage(), "\n";
}

$str = "abc";
$str2 = $str;
sodium_add(inout $str, $addStr);
var_dump($str, $str2);

$str = "ab" . ($x = "c");
$str2 = $str;
sodium_add(inout $str, $addStr);
var_dump($str, $str2);
}

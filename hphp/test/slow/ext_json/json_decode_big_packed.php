<?hh

// Exercises slow-case for packed array construction, and also has the
// current worst-case ratio of encoded vs. in-memory size.
<<__EntryPoint>>
function main_json_decode_big_packed() :mixed{
$arr = vec[];
for ($i = 0; $i <= 9000; $i += 1) {
  $arr[] = $i % 10;
}
$str = json_encode($arr);
var_dump(json_decode($str, true, 512, JSON_FB_LOOSE));
}

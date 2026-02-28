<?hh


<<__EntryPoint>>
function main_asort() :mixed{
$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];

asort(inout $fruits);
var_dump($fruits);

$arr = vec["at", "\xe0s", "as"];
asort(inout $arr, 0);
$arr = vec["num2ber", "num1ber", "num10ber"];
asort(inout $arr, SORT_REGULAR);
var_dump($arr);

$arr = vec["G\xediron",        // &iacute; (Latin-1)
                     "G\xc3\xb3nzales",  // &oacute; (UTF-8)
                     "G\xc3\xa9 ara",    // &eacute; (UTF-8)
                     "G\xe1rcia"];       // &aacute; (Latin-1)
asort(inout $arr, SORT_REGULAR);

// Make the output ASCII-safe
foreach($arr as $key => $val) {
  $arr[$key] = urlencode($val);
}
var_dump($arr);
}

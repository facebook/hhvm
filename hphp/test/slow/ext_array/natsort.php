<?hh


<<__EntryPoint>>
function main_natsort() :mixed{
$array1 = vec["img12.png", "img10.png", "img2.png", "img1.png"];
$array2 = $array1;
sort(inout $array1);
var_dump($array1);

natsort(inout $array2);
var_dump($array2);
}

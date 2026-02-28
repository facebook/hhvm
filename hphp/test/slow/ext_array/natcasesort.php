<?hh


<<__EntryPoint>>
function main_natcasesort() :mixed{
$array1 = vec["IMG0.png", "img12.png", "img10.png",
                "img2.png", "img1.png", "IMG3.png"];
$array2 = $array1;
sort(inout $array1);
var_dump($array1);

natcasesort(inout $array2);
var_dump($array2);
}

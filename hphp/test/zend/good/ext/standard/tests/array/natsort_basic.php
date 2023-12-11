<?hh
/*
* proto bool natsort ( array &$array )
* Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
$array1 = $array2 = vec["img12.png", "img10.png", "img2.png", "img1.png"];
sort(inout $array1);
echo "Standard sorting\n";
print_r($array1);
natsort(inout $array2);
echo "\nNatural order sorting\n";
print_r($array2);
}

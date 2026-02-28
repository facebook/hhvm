<?hh


<<__EntryPoint>>
function main_bin_format() :mixed{
echo "simple\n";
var_dump(0b0);
var_dump(0b1);
var_dump(0b1111);
var_dump(0b0000);
var_dump(0b1000);
var_dump(0b0001);

echo "negative simple\n";
var_dump(-0b0);
var_dump(-0b1);
var_dump(-0b1111);

echo  "simple operations\n";
var_dump(0b10 + 2); // int(4)
var_dump(0b10 - 1); // int(1)
var_dump(0b10 * 2); // int(4)
var_dump(0b11 / 2); // float(1.5)

echo  "large\n";
var_dump(0b11111111111111111111111111111111);
var_dump(0b11111111111111111111111111111111+1);
var_dump(0b111111111111111111111111111111111111111111111111111111111111111);
var_dump(-0b11111111111111111111111111111111);
var_dump(-0b11111111111111111111111111111111-1);
var_dump(-0b111111111111111111111111111111111111111111111111111111111111111);
var_dump(
  -0b111111111111111111111111111111111111111111111111111111111111111 - 1);

// consistent overflow-behavior - THIS CONTRADICTS ZEND's BEHAVIOR
//
// As soon as HHVM is changed to "normal" Zend behavior, these tests
// need to be edited/removed
echo "overflows\n";
var_dump(
  0b111111111111111111111111111111111111111111111111111111111111111 + 1);
var_dump(
  -0b111111111111111111111111111111111111111111111111111111111111111 - 2);

// make sure 0b overflows like normal integers and 0x hexadecimal integer
echo "overflow consistency\n";
$ofBin = 0b111111111111111111111111111111111111111111111111111111111111111 + 1;
$ofInt = 9223372036854775807 + 1;
$ofHex = 0x7FFFFFFFFFFFFFFF + 1;
$ufBin =
  -0b111111111111111111111111111111111111111111111111111111111111111 - 2;
$ufInt = -9223372036854775807 - 2;
$ufHex = -0x7FFFFFFFFFFFFFFF - 2;

var_dump($ofBin === $ofInt);
var_dump($ofBin === $ofHex);
var_dump($ufBin === $ufInt);
var_dump($ufBin === $ufHex);

echo "array-index\n";
$array = vec[0, 1, 2, 3];
var_dump($array[0b11]);

// PHP5 does not have "real" runtime support as of 5.5.7
// So the expected behavior is not what you would expect "logically"
echo "runtime support\n";
$i = (int)    "0b100";
$f = (float)  "0b100";
var_dump($i);                        // int(0)
var_dump($f);                        // float(0)
var_dump(is_numeric("0b100"));       // false
}

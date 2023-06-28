<?hh


<<__EntryPoint>>
function main_1430() :mixed{
$a = (int)("123.456") % 123;
var_dump($a);
$a = (int)("123.456") % 456.123;
var_dump($a);
$a = (int)("123.456") % (int)("123");
var_dump($a);
$a = (int)("123.456") % (int)("456.123");
var_dump($a);
$a = "123.456";
$a = (int)($a);
$a %= 123;
var_dump($a);
$a = "123.456";
$a = (int)($a);
$a %= 456.123;
var_dump($a);
$a = "123.456";
$a = (int)($a);
$a %= (int)("123");
var_dump($a);
$a = "123.456";
$a = (int)($a);
$a %= (int)("456.123");
var_dump($a);
$a = (int)("123") % 123;
var_dump($a);
$a = (int)("123") % (int)("123");
var_dump($a);
$a = (int)("321") % 123;
var_dump($a);
$a = (int)("321") % 123.456;
var_dump($a);
}

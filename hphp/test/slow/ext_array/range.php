<?hh


<<__EntryPoint>>
function main_range() :mixed{
$ret = range(0, 12);
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[12]);

// The step parameter was introduced in 5.0.0
$ret = range(0, 100, 10);
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[5]);
var_dump($ret[10]);

// Use of character sequences introduced in 4.1.0
// array("a", "b", "c", "d", "e", "f", "g", "h", "i");
$ret = range("a", "i");
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[4]);
var_dump($ret[8]);

var_dump(range("c", "a"));

var_dump(range(1.0, 3.0));
var_dump(range(3.0, 1.0));

// test large range boundaries. Old versions of php and HHVM internally
// converted range boundaries to floating point numbers, which could couse
// precision loss.
var_dump(range(-9223372036854775298, -9223372036854775294));
var_dump(range('-9223372036854775298', '-9223372036854775294'));
var_dump(range(1<<55, (1<<55) + 20, 10));

// verify, floating point boundaries do not need to infinite loop
$ret = range(-9223372036854775298.0, -9223372036854775294.0);
var_dump(gettype($ret));
var_dump(count($ret) >= 1024);
$ret = range('-9223372036854775298.0', '-9223372036854775294.0');
var_dump(gettype($ret));
var_dump(count($ret) >= 1024);
}

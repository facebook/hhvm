<?hh
class ObjectA
{
  public $foo = 'bar';
}
<<__EntryPoint>> function main(): void {
echo "*** test key_exists() by using mixed type of arrays ***\n";

// there is not a index = 0 element
$a = darray[1 => 'bar', 'foo' => 'baz'];
var_dump(key_exists(0, $a));

echo "integer\n";
// 1 has index = 0
$b = darray[0 => 1, 'foo' => 'baz'];
var_dump(key_exists(0, $b));

// 42 has index = 0, netherless its position is the latest
$c = darray['foo' => 'baz', 0 => 42];
var_dump(key_exists(0, $c));

echo "string\n";
// 'bar' has index = 0, netherless it is a string
$d = darray[0 => 'bar', 'foo' => 'baz'];
var_dump(key_exists(0, $d));

// 'baz' has index = 0, netherless its position is the latest
$e = darray['foo' => 'baz', 0 => 'baz'];
var_dump(key_exists(0, $e));

echo "obj\n";
$obj = new ObjectA();

// object has index = 0, netherless its position is the latest
$f = darray['foo' => 'baz', 0 => $obj];
var_dump(key_exists(0, $f));

// object has index = 0, netherless its position is the first
$g = darray[0 => $obj, 'foo' => 'baz'];
var_dump(key_exists(0, $g));

echo "stream resource\n";
// stream resource has index = 0, netherless its position is the first
$st = fopen('php://memory', '+r');
$h = darray[0 => $st, 'foo' => 'baz'];
var_dump(key_exists(0, $h));

// stream resource has index = 0, netherless its position is the latest
$i = darray['foo' => 'baz', 0 => $st];
var_dump(key_exists(0, $i));
}

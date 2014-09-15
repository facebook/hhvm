<?hh
class T{}

<<__Memoize>>
function testInvalidObject(mixed $a) { return $a; }

echo testInvalidObject(new T());

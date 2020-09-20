<?hh
class T{}

<<__Memoize>>
function testInvalidObject(mixed $a) { return $a; }
<<__EntryPoint>> function main(): void {
echo testInvalidObject(new T());
}

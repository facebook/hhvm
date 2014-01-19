<?hh

type MyInt    = int;
type MyBool   = bool;
type MyDouble = double;
type MyArray  = array;
type MyString = string;

function fooInt(MyInt $x): void {
  echo "int:\n";
  var_dump($x);
}
function fooBool(MyBool $x): void {
  echo "bool\n";
  var_dump($x);
}
function fooDouble(MyDouble $x): void {
  echo "double\n";
  var_dump($x);
}
function fooArray(MyArray $x): void {
  echo "array:\n";
  var_dump($x);
}
function fooString(MyString $x): void {
  echo "string:\n";
  var_dump($x);
}

function main() {
  fooInt(12);
  fooBool(false);
  fooDouble(1.2);
  fooArray(array(1,2,3));
  fooString("asdasd");
}
main();

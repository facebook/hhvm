<?hh

class A {
  static public $a = varray[varray[ 12]];
}
<<__EntryPoint>> function main(): void {
echo A::$a[0][0] . "\n";
}

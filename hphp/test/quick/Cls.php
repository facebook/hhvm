<?hh

class A {
  static public $a = vec[vec[ 12]];
}
<<__EntryPoint>> function main(): void {
echo A::$a[0][0] . "\n";
}

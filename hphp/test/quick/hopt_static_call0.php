<?hh
class AA {
  public $aa1 = 0;
  public static function f1() :mixed{ print "Pass\n"; return 1; }
  // public static function f0() {  AA::f1();  }
  public static function f0() :mixed{  if (AA::f1()) {  echo "aa1 != 0\n";  }  }
}
<<__EntryPoint>> function main(): void {
AA::f0();
}

//// module_A.php
<?hh
new module A {}
//
//// module_B.php
<?hh
new module B {}
//
//// A.php
<?hh
module A;
internal class Base {
  const type TC = string;
  const type TC2 = bool;
  public static function meth1(Base::TC $_):void { }
  public static function meth2(self::TC $_):void { }
  public static function meth3(this::TC $_):void { }
  public static function getTC():self::TC {
    return "A";
  }
  public static function meth4(self::TC2 $_):void { }
}

final class Derived extends Base {
}

//// B.php
<?hh
module B;
function main(): void {
  Derived::meth1(1);
  Derived::meth2(2);
  Derived::meth3(3);
  $x = Derived::getTC();
  // Make sure that distinct type constants aren't compatible
  Derived::meth4($x);
}

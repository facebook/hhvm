<?hh

class Base {
}
trait T {
  public function sayClass() :mixed{
    echo get_class(new self()) . "\n";
  }
  public function sayParent() :mixed{
    echo get_parent_class(self::class);
  }
}
class Cls extends Base {
 use T;
 }
<<__EntryPoint>> function main(): void {
$o = new Cls();
$o->sayClass();
  // echo Cls
$o->sayParent();
 // echos Base
}

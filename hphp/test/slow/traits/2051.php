<?hh

class Base {
}
trait T {
  public function sayClass() :mixed{
    echo get_class() . "\n";
  }
  public function sayParent() :mixed{
    echo get_parent_class();
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

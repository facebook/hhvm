<?hh

trait T {
  public static $x=1;
  public function printX() :mixed{
 var_dump(self::$x);
 }
}
class C1 {
 use T;
 }
class C2 {
 use T;
 }

function error_boundary($fn) :mixed{
  try {
    $fn();
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main_2052() :mixed{
$o1 = new C1;
$o2 = new C2;
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
error_boundary(() ==> T::$x++);
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
error_boundary(() ==> C1::$x++);
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
error_boundary(() ==> C2::$x++);
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
error_boundary(() ==> $o1->x++);
var_dump(T::$x);
var_dump(C1::$x);
var_dump(C2::$x);
$o1->printX();
$o2->printX();
}

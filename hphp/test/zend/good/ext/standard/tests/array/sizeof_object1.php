<?hh
/* Prototype  : int sizeof($mixed var[, int $mode])
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

class sizeof_class implements Countable
{
  public $member1;
  private $member2;
  protected $member3;

  public function count()
:mixed  {
    return 3; // return the count of member variables in the object
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing sizeof() : object functionality ***\n";

echo "-- Testing sizeof() with an object which implements Countable interface --\n";
$obj = new sizeof_class();

echo "-- Testing sizeof() in default mode --\n";
var_dump( sizeof($obj) );

echo "Done";
}

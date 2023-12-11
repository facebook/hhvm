<?hh

///*
class Point
{
    public $x = -10;    // gets applied before constructor runs
    public $y = 10;     // gets applied before constructor runs

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
    }

    public function __toString()
:mixed    {
        return '(' . $this->x . ',' . $this->y . ')';
    }
}

function f() :mixed{ return 10; }

class X
{
//  const Cprop1 = 10 + 12 - 5.6;       // invalid
//  const Cprop2 = f();                 // invalid
//  const Cprop10 = vec[];            // Arrays are not allowed in class constants
//  const Cprop11 = array(10, "red", TRUE);
//  const Cprop12 = array(10, "red", TRUE, f());
//  const Cprop13 = array(10, "red", array(2.3, NULL, array(12, FALSE, "zzz")));

//  private $prop1 = 10 + 12 - 5.6;     // invalid
//  private $prop2 = f();               // invalid
    private $prop10 = vec[];
    private $prop11 = vec[10, "red", TRUE];
//  private $prop12 = array(10, "red", TRUE, f());  // invalid
    private $prop13 = vec[10, "red", vec[2.3, NULL, vec[12, FALSE, "zzz"]]];


    public $q1;         // take on NULL by default
    public static $q2;  // take on NULL by default
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $p = new Point;
  echo $p . "\n";

  $p = new Point();
  echo $p . "\n";

  $p = new Point(100);
  echo $p . "\n";

  $p = new Point(1000, 2000);
  echo $p . "\n";
  //*/

  echo "--------------------\n";

  $x = new X;
  var_dump($x->q1);
  var_dump(X::$q2);
}

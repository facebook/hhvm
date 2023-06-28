<?hh

class C
{
    private $m;
    public function __construct($p1)
    {
        $this->m = $p1;
    }

///*
    public function __clone()
:mixed    {
        echo "Inside " . __METHOD__ . "\n";

//      return NULL;    // ignored; not passed along as the result of 'clone'
    }
//*/
}

class Employee
{
    private $name;

    public function __construct($name)
    {
        $this->name = $name;
    }

    public function __clone()
:mixed    {
        echo "Inside " . __METHOD__ . "\n";
//      $v = parent::__clone(); // as class has no parent, this is diagnosed

        // make a copy of Employee object

        return 999; // ignored; not passed along as the result of 'clone'

    }
}

class Manager extends Employee
{
    private $level;

    public function __construct($name, $level)
    {
        parent::__construct($name);
        $this->level = $level;
    }

    public function __clone()
:mixed    {
        echo "Inside " . __METHOD__ . "\n";

        $v = parent::__clone();
        echo "\n====>>>>"; var_dump($v);

// make a copy of Manager object

//      return 999; // ignored; not passed along as the result of 'clone'

    }
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  echo "================= play around a bit =================\n";

  $obj1 = new C(10);
  var_dump($obj1);

  $obj2 = clone $obj1;    // default action is to make a shallow copy
  var_dump($obj2);

  //$obj3 = $obj1->__clone(); // can't call directly!! Why is that?
  //var_dump($obj3);

  echo "================= Use cloning in Point class =================\n";

  include_once 'Point2.inc';

  $p1 = new Point2;
  var_dump($p1);
  $p2 = clone $p1;
  var_dump($p2);

  var_dump($p3 = clone $p1);

  var_dump($p4 = clone $p1);

  echo "================= use chained cloning in a class hierarchy =================\n";

  $obj3 = new Manager("Smith", 23);
  var_dump($obj3);

  $obj4 = clone $obj3;
  var_dump($obj4);
}

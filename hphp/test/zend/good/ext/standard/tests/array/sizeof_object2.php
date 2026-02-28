<?hh
/* Prototype  : int sizeof($mixed var[, int $mode] )
 * Description: Counts an elements in an array. If Standard PHP library is installed,
 * it will return the properties of an object.
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: count()
 */

// class without member
class test {
  // no members
}

// class with only members and with out member functions
class test1 {
  public $member1;
  public $var1;
  private $member2;
  protected $member3;

  // no member functions
}

// class with only member functions
class test2 {
  // no data members

  public function display() :mixed{
    echo " Class Name : test2\n";
  }
}

// child class which inherits parent test2
class child_test2 extends test2 {
  public $child_member1;
  private $child_member2;
}

// abstract class
abstract class abstract_class {
  public $member1;
  private $member2;

  abstract protected function display():mixed;
}

// implement abstract 'abstract_class' class
class concrete_class extends abstract_class {
  protected function display() :mixed{
    echo " class name is : concrete_class \n ";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "*** Testing sizeof() : object functionality ***\n";

  echo "--- Testing sizeof() with objects which doesn't implement Countable interface ---\n";

  $objects = vec[
  /* 1  */  new test(),
            new test1(),
            new test2(),
            new child_test2(),
  /* 5  */  new concrete_class()
  ];

  $counter = 1;
  for($i = 0; $i < count($objects); $i++) {
    echo "-- Iteration $counter --\n";
    $var = $objects[$i];

    echo "Default Mode: ";
    var_dump(sizeof($var));
    echo "\n";

    $counter++;
  }

  echo "Done";
}

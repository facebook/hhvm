<?hh
/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
   Description: Verify that the contents of a variable can be called as a function
                In case of objects, $var = array($SomeObject, 'MethodName')
*/

/* Prototype: void check_iscallable_objects( $methods );
   Description: use is_callable() on given $method to check if the array
                contains a valid method name;
                returns true if valid function name, false otherwise
*/
function check_iscallable_objects($methods, $loop_counter) :mixed{
  $counter = 1;
  foreach ($methods as $method) {
    echo "-- Innerloop iteration $counter of Outerloop iteration ".$loop_counter." --\n";
    var_dump(is_callable($method));
    var_dump(is_callable($method, true));
    var_dump(is_callable($method, false));
    $callable_name = null;
    var_dump(is_callable_with_name($method, true, inout $callable_name));
    echo $callable_name, "\n";
    var_dump(is_callable_with_name($method, false, inout $callable_name));
    echo $callable_name, "\n";
    $counter++;
  }
}

class object_class {
  public $value = 100;

  /* static method */
  static public function foo() :mixed{
  }

  public function foo1() :mixed{
  }
  /* function name with mixed string and integer */
  public function x123() :mixed{
  }
  /* function name as NULL */
  public function null() :mixed{
  }
  /* function name having boolean value */
  public function TRUE() :mixed{
  }

  protected function foo2() :mixed{
  }
  private function foo3() :mixed{
  }
}

/* class with no member */
class no_member_class {
 // no members
}

/* class with member as object of other class */
class contains_object_class {
   public $class_object1;
   public $no_member_class_object;

   public function func() :mixed{
     echo "func() is called \n";
   }

   function __construct () {
     $this->class_object1 = new object_class();
     $this->no_member_class_object = new no_member_class();
   }
}

<<__EntryPoint>>
function main(): void {
  echo "\n*** Testing is_callable() on objects ***\n";
  /* objects of different classes */
  $obj = new contains_object_class;
  $temp_class_obj = new object_class();

  /* check is_callable() on static method */
  echo "\n** Testing behavior of is_callable() on static methods **\n";
  var_dump(is_callable('object_class::foo()', true)); //expected: true
  var_dump(is_callable('object_class::foo()')); //expected: false

  echo "\n** Testing normal operations of is_callable() on objects **\n";
  $objects = vec[
    new object_class,
    new no_member_class,
    new contains_object_class,
    $obj,
    $obj->class_object1,
    $obj->no_member_class_object,
    $temp_class_obj,
  ];

  /* loop to check whether given object/string has valid given method name
   *  expected: true if valid callback
   *            false otherwise
   */
  $loop_counter = 1;
  foreach ($objects as $object) {
    echo "--- Outerloop iteration ".$loop_counter." ---\n";
    $methods = vec[
      vec[$object, 'foo1'],
      vec[$object, 'foo2'],
      vec[$object, 'foo3'],
      vec[$object, 'x123'],
      vec[$object, 'null'],
      vec[$object, 'TRUE'],
      vec[$object, '123'],
      vec[$temp_class_obj->value, 100],
      vec[$object, 'func'],
      vec['object_class', 'foo1'],
    ];
    /* use check_iscallable_objects() to check whether given object/string
       has valid method name */
    check_iscallable_objects($methods, $loop_counter);
    $loop_counter++;
  }

  echo "===DONE===\n";
}

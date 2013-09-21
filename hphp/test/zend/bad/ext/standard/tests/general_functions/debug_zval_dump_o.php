<?php
/* Prototype: void debug_zval_dump ( mixed $variable );
   Description: Dumps a string representation of an internal zend value to output.
*/

/* Prototype: void zval_dump( $value );
   Description: use debug_zval_dump() to display the objects and its 
                reference count */
function zval_dump( $values ) {
  $counter = 1;
  foreach( $values as $value ) {
    echo "-- Iteration $counter --\n";
    debug_zval_dump( $value );
    $counter++;
  }
}

/* checking on objects type */
echo "*** Testing debug_zval_dump() on objects ***\n";
class object_class {
  var $value1 = 1;
  private $value2 = 10;
  protected $value3 = 20;
  public $value4 = 30;

  private function foo1() {
	echo "function foo1\n";
  }
  protected function foo2() {
	echo "function foo2\n";
  }
  public function foo3() {
	echo "function foo3\n";
  }
  public $array_var  = array( "key1" => 1, "key2 " => 3);

  function object_class () {
      $this->value1 = 5;
      $this->object_class1 = $this;
  }
}

class no_member_class{
//no members
}

/* class with member as object of other class */
class contains_object_class
{
   var       $p = 30;
   protected $p1 = 40;
   private   $p2 = 50;
   var       $class_object1;
   public    $class_object2;
   private   $class_object3;
   protected $class_object4;
   var       $no_member_class_object;

   public function func() {
     echo "func() is called \n";
   }

   function contains_object_class () {
     $this->class_object1 = new object_class();
     $this->class_object2 = new object_class();
     $this->class_object3 = $this->class_object1;
     $this->class_object4 = $this->class_object2;
     $this->no_member_class_object = new no_member_class();
     $this->class_object5 = $this;   //recursive reference
   }
}

/* creating new object $obj */
$obj = new contains_object_class(); 
$obj1 = & $obj;  //object $obj1 references object $obj
$obj2 = & $obj;
$obj3 = & $obj2;

/* object which is unset */
$unset_obj = new object_class();
unset($unset_obj);

$objects = array (
  new object_class,
  new no_member_class,
  $obj,
  $obj->class_object1,
  $obj->class_object2,
  $obj->no_member_class_object,
  @$temp_class_obj,  //undefined object
  $obj2->class_object1,
  $obj3->class_object2,
  $obj2->class_object1->value4,
  @$unset_obj
);
/* using zval_dump() to dump out the objects and its reference count */
zval_dump($objects);

$int_var = 500;
$obj = $int_var;  //$obj is lost, $obj1,$obj2,$obj3,$obj4 = 500
echo "\n-- Testing debug_zval_dump() on overwritten object variables --\n";
debug_zval_dump($obj, $obj1, $obj2, $obj3);

echo "\n-- Testing debug_zval_dump() on objects having circular reference --\n";
$recursion_obj1 = new object_class();
$recursion_obj2 = new object_class();
$recursion_obj1->obj = &$recursion_obj2;  //circular reference
$recursion_obj2->obj = &$recursion_obj1;  //circular reference
debug_zval_dump($recursion_obj2);

echo "Done\n";
?>
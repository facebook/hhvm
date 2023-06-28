<?hh
/* Prototype: int count ( mixed $var [, int $mode] );
 * Description: Count elements in an array, or properties in an object
 */
class count_class implements Countable {
  private $var_private;
  public $var_public;
  protected $var_protected;

  public function count() :mixed{
    return 3;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing basic functionality of count() function ***\n";
print "-- Testing NULL --\n";
$arr = NULL;
print "COUNT_NORMAL: should be 0, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 0, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing arrays --\n";
$arr = varray[1, varray[3, 4, varray[6, varray[8]]]];
print "COUNT_NORMAL: should be 2, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 8, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing hashes --\n";
$arr = darray["a" => 1, "b" => 2, 0 => darray["c" => 3, 0 => darray["d" => 5]]];
print "COUNT_NORMAL: should be 3, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 6, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing strings --\n";
print "COUNT_NORMAL: should be 1, is ".count("string", COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 1, is ".count("string", COUNT_RECURSIVE)."\n";

print "-- Testing various types with no second argument --\n";
print "COUNT_NORMAL: should be 1, is ".count("string")."\n";
print "COUNT_NORMAL: should be 2, is ".count(varray["a", varray["b"]])."\n";

$arr = darray['a'=>varray[NULL, NULL, NULL], 1=>darray[''=>1, 1=>NULL],
    2 => varray[varray[varray[varray[varray[NULL]]]]]];
print "-- Testing really cool arrays --\n";
print "COUNT_NORMAL: should be 3, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 13, is ".count($arr, COUNT_RECURSIVE)."\n";

echo "\n*** Testing possible variations of count() function on arrays ***";
$count_array = varray[
  varray[],
  darray[ 1 => "string"],
  darray[ "" => "string", 0 => "a", '' => "b", -1 => "c",
         1 => varray[varray[varray[NULL]]]],
  darray[ -2 => 12, 0 => varray[varray[1, 2, varray[varray["0"]]]]],
  darray[ "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL, "d" => -2.344],
  darray[ 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL,
         1 => -2.344, 5 => varray[]],
  darray[ 1 => TRUE, 0 => FALSE, "" => "", " " => " ",
     '' => NULL, "\x000" => "\x000", "\000" => "\000"],
  darray[ 0 => NULL, 1 => "Hi", "string" => "hello",
          2 => darray["" => "World", "-2.34" => "a", "0" => "b"]]
];

$i = 0;
foreach ($count_array as $count_value) {
  echo "\n-- Iteration $i --\n";
  print "COUNT_NORMAL is ".count($count_value, COUNT_NORMAL)."\n";
  print "COUNT_RECURSIVE is ".count($count_value, COUNT_RECURSIVE)."\n";
  $i++;
}


/* Testing count() by passing constant with no second argument */
print "\n-- Testing count() on constants with no second argument --\n";
print "COUNT_NORMAL: should be 1, is ".count(100)."\n";
print "COUNT_NORMAL: should be 1, is ".count(-23.45)."\n";

print "\n-- Testing count() on NULL and Unset variables --\n";
print "COUNT_NORMAL: should be 0, is ".count(NULL)."\n";
print "COUNT_NORMAL: should be 1, is ".count("")."\n";
try {print "COUNT_NORMAL: should be 0, is ".@count($a)."\n";} catch (UndefinedVariableException $e) {var_dump($e->getMessage());}


print "\n-- Testing count() on an empty sub-array --\n";
$arr = varray[1, varray[3, 4, varray[]]];
print "COUNT_NORMAL: should be 2, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 5, is ".count($arr, COUNT_RECURSIVE)."\n";

echo "\n-- Testing count() on objects with Countable interface --\n";

$obj = new count_class();
print "COUNT_NORMAL: should be 3, is ".count($obj)."\n";


echo "\n-- Testing count() on resource type --\n";
$resource1 = fopen( __FILE__, "r" );  // Creating file(stream type) resource
$resource2 = opendir( "." );  // Creating dir resource

/* creating an array with resources as elements */
$arr_resource = darray["a" => $resource1, "b" => $resource2];
var_dump(count($arr_resource));

echo "\n-- Testing error conditions --";
try { var_dump( count() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // No. of args = 0
try { var_dump( count(varray[], COUNT_NORMAL, 100) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // No. of args > expected

/* Testing Invalid type arguments */
try { var_dump( count(100, "string") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( count(varray[], "") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\nDone";

/* closing the resource handles */
fclose( $resource1 );
closedir( $resource2 );
}

<?hh
/* Prototype  : string get_resource_type  ( resource $handle  )
 * Description:  Returns the resource type
 * Source code: Zend/zend_builtin_functions.c
 */

class Hello {
  public function SayHello($arg) :mixed{
      echo "Hello\n";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing get_resource_type() : variation test ***\n";

$res = fopen(__FILE__, "r");

$vars = dict[
    "bool"=>true,
    "int 10"=>10,
    "float 10.5"=>10.5,
    "string"=>"Hello World",
    "array"=>vec[1,2,3,4,5],
    "NULL"=>NULL,
    "Object"=>new Hello()
];

foreach($vars as $variation =>$object) {
      echo "\n-- $variation --\n";
      try { var_dump(get_resource_type($object)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "===DONE===\n";
}

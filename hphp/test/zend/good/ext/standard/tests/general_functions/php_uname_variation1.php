<?hh
/* Prototype: string php_uname  ([ string $mode  ] )
 * Description:  Returns information about the operating system PHP is running on
*/

class fooClass {
   function __toString() :mixed{
       return "m";
   }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing php_uname() - usage variations\n";
// Prevent notices about undefines variables
error_reporting(E_ALL & ~E_NOTICE);


$values = dict[
          // empty data
          "\"\"" => "",
          "''" => '',
];

// loop through each element of the array for data

foreach($values as $key => $value) {
      echo "-- Iterator $key --\n";
      try { var_dump( php_uname($value) ); } catch (Exception $e) { echo 'ERROR: '; var_dump($e->getMessage()); }
};

echo "===DONE===\n";
}

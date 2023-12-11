<?hh


interface Interf1 { }
interface Interf2 { }
interface Interf3 { }

abstract class AC1 implements Interf2, Interf1 { }
abstract class AC2 extends AC1 { }
abstract class AC3 extends AC2 implements Interf3 { }
abstract class AC4 extends AC3 { }

class C1 extends AC4 { }

function array_some(darray $array) :mixed{
  foreach ($array as $value) {
    if ($value) {
      echo "Empty: ";
      echo !($value ?? false);
      echo "\nBool: ";
      echo $value ? "true" : "false";
      echo "\n";
    }
  }
  return "Done";
}
<<__EntryPoint>> function main(): void {
$a = new C1;

var_dump( array_some(array_map(function($v) { return $v is Interf2; },
                     vec[$a])) );
}

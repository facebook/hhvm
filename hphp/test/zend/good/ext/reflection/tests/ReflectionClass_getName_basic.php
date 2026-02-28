<?hh
class TrickClass {
    function __toString() :mixed{
        //Return the name of another class
        return "Exception";
    }
}
<<__EntryPoint>> function main(): void {
$r1 = new ReflectionClass("stdClass");

$myInstance = new stdClass;
$r2 = new ReflectionClass($myInstance);

$r3 = new ReflectionClass("TrickClass");

var_dump($r1->getName(), $r2->getName(), $r3->getName());
}

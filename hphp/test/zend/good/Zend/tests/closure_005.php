<?hh

class A {
    private $x;

    function __construct($x) {
        $this->x = $x;
    }

    function getIncer($val) {
        return function() use ($val) {
            $this->x += $val;
        };
    }

    function getPrinter() {
        return function() {
            echo $this->x."\n";
        };
    }







    function printX() {
        echo $this->x."\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new A(3);
$incer = $a->getIncer(2);
$printer = $a->getPrinter();


$a->printX();
$printer();
$incer();
$a->printX();
$printer();

unset($a);

$incer();
$printer();

unset($incer);
$printer();

unset($printer);



echo "Done\n";
}

<?hh

class A {
    private $x = 0;

    function getClosureGetter () {
        return function () {
            return function () {
                $this->x++;
            };
        };
    }

    function printX () {
        echo $this->x."\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new A;
$a->printX();
$getClosure = $a->getClosureGetter();
$a->printX();
$closure = $getClosure();
$a->printX();
$closure();
$a->printX();

echo "Done\n";
}

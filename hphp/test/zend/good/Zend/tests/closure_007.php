<?hh

class A {
    private $x = 0;

    function getClosureGetter () :mixed{
        return function () {
            return function () {
                $this->x++;
            };
        };
    }

    function printX () :mixed{
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

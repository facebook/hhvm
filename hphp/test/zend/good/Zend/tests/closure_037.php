<?hh
class A {
    private $x = 0;

    function getClosure () :mixed{
            return function () {
                $this->x++;
                self::printX();
                self::print42();
                static::print42();
            };
    }

    function printX () :mixed{
        echo $this->x."\n";
    }

    function print42() :mixed{
        echo "42\n";
    }
}

class B extends A {
    function print42() :mixed{
        echo "forty two\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = new A;
$closure = $a->getClosure();
$closure();
$b = new B;
$closure = $b->getClosure();
$closure();
echo "Done.";
}

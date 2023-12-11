<?hh

abstract class first {
    protected $someArray = vec[];
}

class second extends first {
    protected $someArray = vec[];
    protected $someValue = null;

    public function __construct($someValue) {
        $this->someValue = $someValue;
    }
}
<<__EntryPoint>> function main(): void {
$objFirst = new second('123');
$objSecond = new second('321');

var_dump ($objFirst == $objSecond);
}

<?hh
class par {
    private $id = "foo";

    function displayMe()
:mixed    {
        print $this->id;
    }
}

class chld extends par {
    public $id = "bar";
    function displayHim()
:mixed    {
        parent::displayMe();
    }
}

<<__EntryPoint>> function main(): void {
$obj = new chld();
$obj->displayHim();
}

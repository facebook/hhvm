<?hh
class par {
    private $id = "foo";

    function displayMe()
    {
        print $this->id;
    }
};

class chld extends par {
    public $id = "bar";
    function displayHim()
    {
        parent::displayMe();
    }
};

<<__EntryPoint>> function main(): void {
$obj = new chld();
$obj->displayHim();
}

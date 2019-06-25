<?hh

class par {
    private $id="foo";

    function displayMe()
    {
        $this->displayChild();
    }
};

class chld extends par {
    private $id = "bar";

    function displayChild()
    {
        print $this->id;
    }
};

<<__EntryPoint>> function main(): void {
$obj = new chld();
$obj->displayMe();
}

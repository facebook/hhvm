<?hh

class par {
    private $id="foo";

    function displayMe()
:mixed    {
        $this->displayChild();
    }
}

class chld extends par {
    private $id = "bar";

    function displayChild()
:mixed    {
        print $this->id;
    }
}

<<__EntryPoint>> function main(): void {
$obj = new chld();
$obj->displayMe();
}

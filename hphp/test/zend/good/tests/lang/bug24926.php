<?hh

class foo {

    public $functions = dict[];

    function __construct()
    {
        $function = () ==> "FOO\n";
        print($function());

        $this->functions['test'] = $function;
        print($this->functions['test']());    // werkt al niet meer

    }
}

<<__EntryPoint>> function main(): void {
error_reporting (E_ALL);

$a = new foo ();
}

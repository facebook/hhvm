<?hh
class Pwa {
    public $var;

    function __construct(){
        $this->var = darray[];
    }

    function test (){
        $cont = darray[];
        $cont["mykey"] = "myvalue";

        foreach ($cont as $this->var['key'] => $this->var['value'])
        var_dump($this->var['value']);
    }
}
<<__EntryPoint>> function main(): void {
$myPwa = new Pwa();
$myPwa->test();

echo "Done\n";
}

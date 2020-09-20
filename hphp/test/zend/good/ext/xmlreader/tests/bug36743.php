<?hh

class Test extends XMLReader
{
    private $testArr = varray[];
    public function __construct()
    {
        $this->testArr[] = 1;
        var_dump($this->testArr);
    }
}
<<__EntryPoint>> function main(): void {
$t = new test;

echo "Done\n";
}

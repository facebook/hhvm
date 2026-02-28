<?hh
class myZip extends ZipArchive {
    private $test = 0;
    public $testp = 1;
    private $testarray = vec[];

    public function __construct() {
        $this->testarray[] = 1;
        var_dump($this->testarray);
    }
}
<<__EntryPoint>> function main(): void {
$z = new myZip;
}

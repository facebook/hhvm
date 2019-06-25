<?hh

class test1 {
    protected $var;
}

class test extends test1 {
    private $var;
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}

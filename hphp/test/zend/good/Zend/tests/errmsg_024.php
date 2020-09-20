<?hh

class test1 {
    static protected $var = 1;
}

class test extends test1 {
    static $var = 10;
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}

<?hh

class test {
    private $var1 = 1;
    public $var2 = 2;
    protected $var3 = 3;

    function  __toString() {
        return "10";
    }
}
<<__EntryPoint>> function main(): void {
$r = fopen(__FILE__, "r");

$o = new test;

$vars = varray[
    "string",
    "",
    "\0",
    "8754456",
    9876545,
    0.10,
    varray[],
    varray[1,2,3],
    false,
    true,
    NULL,
    $r,
    $o
];

foreach ($vars as $var) {
    $tmp = (array)$var;
    var_dump($tmp);
}

echo "Done\n";
}

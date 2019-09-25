<?hh

class test {
    function  __toString() {
        return "10";
    }
}
<<__EntryPoint>> function main(): void {
$r = fopen(__FILE__, "r");

$o = new test;

$vars = array(
    "string",
    "8754456",
    "",
    "\0",
    9876545,
    0.10,
    array(),
    array(1,2,3),
    false,
    true,
    NULL,
    $r,
    $o
);

foreach ($vars as $var) {
    $tmp = (float)$var;
    var_dump($tmp);
}

echo "Done\n";
}

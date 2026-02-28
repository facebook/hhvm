<?hh

class test {
    function  __toString() :mixed{
        return "10";
    }
}
<<__EntryPoint>> function main(): void {
$r = fopen(__FILE__, "r");

$o = new test;

$vars = vec[
    "string",
    "8754456",
    "",
    "\0",
    9876545,
    0.10,
    vec[],
    vec[1,2,3],
    false,
    true,
    NULL,
    $r,
    $o
];

foreach ($vars as $var) {
    try { $tmp = (string)$var; } catch (Exception $e) { $tmp = $e->getMessage(); }
    var_dump($tmp);
}

echo "Done\n";
}

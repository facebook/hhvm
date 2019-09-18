<?hh

function cmp($a, $b) {
    is_array ($a)
        && ($a = array_sum ($a));
    is_array ($b)
        && ($b = array_sum ($b));
    return strcmp ((string)$a, (string)$b);
}

<<__EntryPoint>> function main(): void {
require(dirname(__FILE__) . '/data.inc');

echo " -- Testing uasort() -- \n";
uasort(&$data, fun('cmp'));
var_dump($data);

echo "\n -- Testing uksort() -- \n";
uksort(&$data, fun('cmp'));
var_dump($data);

echo "\n -- Testing usort() -- \n";
usort(&$data, fun('cmp'));
var_dump($data);
}

<?hh <<__EntryPoint>> function main(): void {
$ds = array(
        0 => array("pipe", "r"),
        1 => array("pipe", "w"),
        2 => array("pipe", "w")
        );

$pipes = null;
$cat = proc_open("/bin/cat", $ds, inout $pipes);

proc_close($cat);

echo "I didn't segfault!\n";
}

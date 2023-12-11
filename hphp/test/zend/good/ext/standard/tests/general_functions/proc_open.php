<?hh <<__EntryPoint>> function main(): void {
$ds = dict[
        0 => vec["pipe", "r"],
        1 => vec["pipe", "w"],
        2 => vec["pipe", "w"]
        ];

$pipes = null;
$cat = proc_open("/bin/cat", $ds, inout $pipes);

proc_close($cat);

echo "I didn't segfault!\n";
}

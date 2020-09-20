<?hh <<__EntryPoint>> function main(): void {
$ds = darray[
        0 => varray["pipe", "r"],
        1 => varray["pipe", "w"],
        2 => varray["pipe", "w"]
        ];

$pipes = null;
$cat = proc_open("/bin/cat", $ds, inout $pipes);

proc_close($cat);

echo "I didn't segfault!\n";
}

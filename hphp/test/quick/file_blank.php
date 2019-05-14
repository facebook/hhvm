<?hh <<__EntryPoint>> function main() {
$fname = tempnam(__DIR__, "blank");
touch($fname);
var_dump(file($fname));
unlink($fname);
}

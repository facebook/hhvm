<?hh <<__EntryPoint>> function main(): void {
$x = "bug";
var_dump(isset($x[-1]));
var_dump(isset($x["1"]));
echo $x["1"]."\n";
}

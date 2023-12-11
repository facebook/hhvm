<?hh <<__EntryPoint>> function main(): void {
$x = dict["default"=>"ok"];
var_dump($x);
$cf = $x;
unset($cf['default']);
var_dump($x);
echo "ok\n";
}

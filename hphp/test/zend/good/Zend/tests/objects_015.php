<?hh
<<__EntryPoint>> function main(): void {
$o=new stdClass;

var_dump($o == "");
var_dump($o != "");
var_dump(HH\Lib\Legacy_FIXME\lt($o, ""));
var_dump(HH\Lib\Legacy_FIXME\lt("", $o));
var_dump(HH\Lib\Legacy_FIXME\gt("", $o));
var_dump($o != null);
var_dump(is_null($o));

echo "===DONE===\n";
}

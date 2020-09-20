<?hh <<__EntryPoint>> function main(): void {
$obj = new stdClass();
$obj->{1} = 2;
$obj->foo = "bar";
foreach ($obj as $name => $value)  {
    echo "$name -> $value\n";
}
}

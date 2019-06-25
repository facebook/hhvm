<?hh <<__EntryPoint>> function main(): void {
$array = array(1 => 2, "foo" => "bar");
$obj = (object)$array;
foreach ($obj as $name => $value)  {
    echo "$name -> $value\n";
}
}

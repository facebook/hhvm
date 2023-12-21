<?hh <<__EntryPoint>> function main(): void {
$a = dict[ "\x90" => 10 ];
foreach ($a as $val=>$key) echo $key;
echo "\nDone\n";
}

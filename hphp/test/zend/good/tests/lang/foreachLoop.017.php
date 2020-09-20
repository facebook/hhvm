<?hh <<__EntryPoint>> function main(): void {
$a = darray [ "\x90" => 10 ];
foreach ($a as $val=>$key) echo $key;
echo "\nDone\n";
}

<?hh <<__EntryPoint>> function main(): void {
$f = fopen("php://fd/1", "wb");
fwrite($f, "hi!");

echo "\nDone.\n";
}

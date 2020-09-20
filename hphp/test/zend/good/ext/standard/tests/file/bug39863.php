<?hh
<<__EntryPoint>> function main(): void {
$filename = __FILE__ . chr(0). ".ridiculous";

if (file_exists($filename)) {
    echo "FAIL\n";
}
else {
    echo "PASS\n";
}
echo "===DONE===\n";
}

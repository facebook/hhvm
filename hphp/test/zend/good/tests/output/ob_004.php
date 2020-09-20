<?hh <<__EntryPoint>> function main(): void {
ob_start();
echo "foo\n";
ob_clean();
echo "bar\n";
}

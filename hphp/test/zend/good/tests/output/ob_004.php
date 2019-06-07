<?hh <<__EntryPoint>> function main() {
ob_start();
echo "foo\n";
ob_clean();
echo "bar\n";
}

<?hh <<__EntryPoint>> function main(): void {
for ($i = 0; $i < 3; ++$i) {
    if (@iconv('blah', 'blah', 'blah') != '') {
        die("failed\n");
    }
}
echo "success\n";
}

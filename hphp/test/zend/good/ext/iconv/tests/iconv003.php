<?hh <<__EntryPoint>> function main() {
for ($i = 0; $i < 3; ++$i) {
    if (@iconv('blah', 'blah', 'blah') != '') {
        die("failed\n");
    }
}
echo "success\n";
}

<?hh <<__EntryPoint>> function main(): void {
for ($i = 0; $i < 3; ++$i) {
    if (HH\Lib\Legacy_FIXME\neq(@iconv('blah', 'blah', 'blah'), '')) {
        exit("failed\n");
    }
}
echo "success\n";
}

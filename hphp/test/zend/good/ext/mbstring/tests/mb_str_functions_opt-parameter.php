<?hh <<__EntryPoint>> function main(): void {
echo mb_strpos('abb', 'b', 0, 'UTF-8') . "\n";
echo mb_strrpos('abb', 'b', 0, 'UTF-8') . "\n";
echo mb_stripos('abb', 'B', 0, 'UTF-8') . "\n";
echo mb_strripos('abb', 'B', 0, 'UTF-8') . "\n";
echo mb_strstr('foobarbaz', 'ba', false, 'UTF-8') . "\n";
echo mb_strrchr('foobarbaz', 'ba', false, 'UTF-8') . "\n";
echo mb_stristr('foobarbaz', 'BA', false, 'UTF-8') . "\n";
echo mb_strrichr('foobarbaz', 'BA', false, 'UTF-8') . "\n";
echo mb_substr('foobarbaz', 6, null, 'UTF-8') . "\n";
echo mb_strcut('foobarbaz', 6, null, 'UTF-8') . "\n";
echo mb_strimwidth('foobar', 0, 3, null, 'UTF-8') . "\n";
echo "==DONE==";
}

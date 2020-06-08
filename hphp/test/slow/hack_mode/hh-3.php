<?hh


<<__EntryPoint>>
function main_hh_3() {
eval(<<<'EOD'
function foo(Vector<int> $a) {
}
function bar(string $x) {
echo $x . "\n";
}
EOD
);
bar("Done");
}

<?hh
<<__EntryPoint>> function main(): void {
$v = 0.005;
for ($i = 1; $i < 10; $i++) {
    $v__str = (string)($v);
    echo "round({$v__str}, 2) -> ".(string)(round($v, 2))."\n";
    $v += 0.01;
}
}

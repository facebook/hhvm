<?hh <<__EntryPoint>> function main(): void {
$v = 1;
for ($i = 1; $i < 10; $i++) {
    $v /= 10;
    $v__str = (string)($v);
    echo "{$v__str} ".(string)(unserialize(serialize($v)))."\n";
}
}

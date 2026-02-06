<?hh <<__EntryPoint>> function main(): void {
$lambda = function () use ($i) {
    ++$i;
    return $i;
};
$lambda();
$lambda();
var_dump($i);
}

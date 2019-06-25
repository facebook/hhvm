<?hh <<__EntryPoint>> function main(): void {
$lambda = function () use ($i) {
    return ++$i;
};
$lambda();
$lambda();
var_dump($i);
}

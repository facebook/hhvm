<?hh <<__EntryPoint>> function main(): void {
$i = 1;
$lambda = function () use ($i) {
    ++$i;
    return $i;
};
$lambda();
echo $lambda()."\n";
//early prototypes gave 3 here because $i was static in $lambda
}

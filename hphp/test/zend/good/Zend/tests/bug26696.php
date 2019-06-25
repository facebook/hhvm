<?hh <<__EntryPoint>> function main(): void {
$str = 'asdd/?';
$len = strlen($str);
for ($i = 0; $i < $len; $i++) {
    switch ($str[$i]) {
    case '?':
        echo "?+\n";
        break;
    default:
        echo $str[$i].'-';
        break;
    }
}

echo "===DONE===\n";
}

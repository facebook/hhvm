<?hh <<__EntryPoint>> function main(): void {
$x = 'abc';
switch ($x[0]) {
    case 'a':
        echo "no crash\n";
        break;
}
}

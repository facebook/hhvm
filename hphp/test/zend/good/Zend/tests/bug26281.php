<?hh <<__EntryPoint>> function main() {
$x = 'abc';
switch ($x[0]) {
    case 'a':
        echo "no crash\n";
        break;
}
}

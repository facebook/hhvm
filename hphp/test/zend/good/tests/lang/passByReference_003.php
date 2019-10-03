<?hh
function passbyVal($val) {
    echo "\nInside passbyVal call:\n";
    var_dump($val);
}

function passbyRef(inout $ref) {
    echo "\nInside passbyRef call:\n";
    var_dump($ref);
}
<<__EntryPoint>> function main(): void {
echo "\nPassing undefined by value\n";
passbyVal($undef1[0]);
echo "\nAfter call\n";
var_dump($undef1);

echo "\nPassing undefined by reference\n";
passbyRef(inout $undef2);
echo "\nAfter call\n";
var_dump($undef2);
}

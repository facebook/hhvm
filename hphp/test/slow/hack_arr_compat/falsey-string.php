<?hh

<<__EntryPoint>>
function main() {
    echo "--------- set --------\n";
    $a = "";
    $a[42] = "hello";
    echo "--------- nested set --------\n";
    $b = "";
    $b[2][4] = "goodbye";
    echo "--------- append --------\n";
    $c = "";
    $c[] = "blargh";
    echo "--------- isset --------\n";
    $c = "";
    var_dump(isset($c[42]));
    echo "--------- query --------\n";
    $c = "";
    var_dump($c[42]);
}

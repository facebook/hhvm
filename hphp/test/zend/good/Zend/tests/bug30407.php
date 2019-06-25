<?hh

function haricow($a = 'one') {
    var_dump($a);
    $a = 'two';
}
<<__EntryPoint>> function main(): void {
haricow();
haricow();
echo "===DONE===\n";
}

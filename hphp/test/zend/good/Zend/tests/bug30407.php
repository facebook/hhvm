<?hh

function haricow($a = 'one') :mixed{
    var_dump($a);
    $a = 'two';
}
<<__EntryPoint>> function main(): void {
haricow();
haricow();
echo "===DONE===\n";
}

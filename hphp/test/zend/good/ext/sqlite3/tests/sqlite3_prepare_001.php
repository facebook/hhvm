<?hh

function test(inout $x) :mixed{
    $class = new SQLite3(':memory:');
    $x = $class->prepare('SELECT 1');
}
<<__EntryPoint>> function main(): void {
$foo = null;
test(inout $foo);

echo "done\n";
}

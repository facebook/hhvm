<?hh <<__EntryPoint>> function main(): void {
$count = 0;
foreach(new FilesystemIterator(__DIR__) as $ent)
{
    ++$count;
}
var_dump($count > 0);
echo "===DONE===\n";
}

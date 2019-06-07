<?hh <<__EntryPoint>> function main() {
try {
    new RecursiveTreeIterator(new ArrayIterator(array()));
} catch (InvalidArgumentException $e) {
    echo "InvalidArgumentException thrown\n";
}
echo "===DONE===\n";
}

<?hh
class myAppendIterator extends AppendIterator {}
<<__EntryPoint>> function main(): void {
try {
    $it = new myAppendIterator();
    echo "no exception";
} catch (InvalidArgumentException $e) {
    echo 'InvalidArgumentException thrown';
}
}

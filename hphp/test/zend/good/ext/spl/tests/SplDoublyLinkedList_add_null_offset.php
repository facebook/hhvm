<?hh <<__EntryPoint>> function main() {
try {
    $dll = new SplDoublyLinkedList();
    var_dump($dll->add(NULL,2));
} catch (OutOfRangeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
}

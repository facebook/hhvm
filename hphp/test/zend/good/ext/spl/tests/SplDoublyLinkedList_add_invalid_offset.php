<?hh <<__EntryPoint>> function main(): void {
try {
    $dll = new SplDoublyLinkedList();
    var_dump($dll->add(12,'Offset 12 should not exist'));
} catch (OutOfRangeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
}

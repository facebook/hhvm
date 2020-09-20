<?hh
    $objDoc = new DomDocument();

    $objRef = $objDoc->createEntityReference('Test');
    echo $objRef->nodeName . "\n";
<<__EntryPoint>> function main(): void {
echo "===DONE===\n";
}

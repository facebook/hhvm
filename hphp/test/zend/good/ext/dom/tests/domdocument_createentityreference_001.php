<?hh
<<__EntryPoint>> function main(): void {
  $objDoc = new DomDocument();

  $objRef = $objDoc->createEntityReference('Test');
  echo $objRef->nodeName . "\n";
  echo "===DONE===\n";
}

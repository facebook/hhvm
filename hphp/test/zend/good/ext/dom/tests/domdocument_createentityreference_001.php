<?hh
<<__EntryPoint>> function main(): void {
  $objDoc = new DOMDocument();

  $objRef = $objDoc->createEntityReference('Test');
  echo $objRef->nodeName . "\n";
  echo "===DONE===\n";
}

<?hh

function basicXML() :mixed{
  return
'<root>
  <a><a1>a1</a1></a>
  <b>b</b>
  <c><c1>c1</c1></c>
  <d>d</d>
</root>';
}

function testInitialize() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "SimpleXMLIterator can be initialized :)\n";
}

function testMeta() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "SimpleXMLIterator is a SimpleXMLElement: ";
  var_export($s is SimpleXMLElement);echo "\n";

  echo "SimpleXMLIterator is a RecursiveIterator: ";
  var_export($s is RecursiveIterator);echo "\n";
}

function testGetBasicProperty() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "Node root/b has contents: {$s->b}\n";
}

function testFlatIteration() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "Iteration of elements: ";
  foreach ($s as $e) {
    echo "$e, ";
  }
  echo "\n";
}

function testChildCountDuringIteration() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "Counts of first level nodes: ";
  foreach ($s as $i => $e) {
    echo "$i: {$s->getChildren()->count()}, ";
  }
  echo "\n";
}

function testHasChildrenDuringIteration() :mixed{
  $s = new SimpleXMLIterator(basicXML());
  echo "first level nodes have children: ";
  foreach ($s as $i => $e) {
    echo "$i: ", ($s->hasChildren() ? 'yes' : 'no'), ", ";
  }
  echo "\n";
}

function testRecursiveIteration() :mixed{
  $s = new RecursiveIteratorIterator(new SimpleXMLIterator(basicXML()));
  echo "Leaves:\n";
  foreach ($s as $i => $e) {
    echo "$e, ";
  }
  echo "\n";
}

<<__EntryPoint>> function main(): void {
testInitialize();

testMeta();

testGetBasicProperty();

testFlatIteration();

testChildCountDuringIteration();

testHasChildrenDuringIteration();

testRecursiveIteration();
}

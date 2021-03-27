<?hh

function basicXML() {
  return
'<root>
  <a><a1>a1</a1></a>
  <b>b</b>
  <c><c1>c1</c1></c>
  <d>d</d>
</root>';
}

function testInitialize() {
  $s = new SimpleXMLIterator(basicXML());
  echo "SimpleXMLIterator can be initialized :)\n";
}

function testMeta() {
  $s = new SimpleXMLIterator(basicXML());
  echo "SimpleXMLIterator is a SimpleXMLElement: ";
  var_export($s is SimpleXMLElement);echo "\n";

  echo "SimpleXMLIterator is a RecursiveIterator: ";
  var_export($s is RecursiveIterator);echo "\n";
}

function testGetBasicProperty() {
  $s = new SimpleXMLIterator(basicXML());
  echo "Node root/b has contents: {$s->b}\n";
}

function testFlatIteration() {
  $s = new SimpleXMLIterator(basicXML());
  echo "Iteration of elements: ";
  foreach ($s as $e) {
    echo "$e, ";
  }
  echo "\n";
}

function testChildCountDuringIteration() {
  $s = new SimpleXMLIterator(basicXML());
  echo "Counts of first level nodes: ";
  foreach ($s as $i => $e) {
    echo "$i: {$s->getChildren()->count()}, ";
  }
  echo "\n";
}

function testHasChildrenDuringIteration() {
  $s = new SimpleXMLIterator(basicXML());
  echo "first level nodes have children: ";
  foreach ($s as $i => $e) {
    echo "$i: ", ($s->hasChildren() ? 'yes' : 'no'), ", ";
  }
  echo "\n";
}

function testRecursiveIteration() {
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

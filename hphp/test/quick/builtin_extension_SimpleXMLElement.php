<?hh

class A_SimpleXMLElement extends SimpleXMLElement {
  public $___x;
}

function basicXML() {
  return
'<root>
  <a><a1>a1</a1></a>
  <b>b</b>
  <c>c1</c>
</root>';
}

function testIteration() {
  $s = new SimpleXMLElement(basicXMl());
  $i = 1;
  foreach ($s as $e) {
    echo 'Of basic XML structure the ';
    echo ($i++) . "th nodes contents is: $e\n";
  }
}

function testCanBeWrappedByIteratorIterator() {
  $i = new IteratorIterator(new SimpleXMLElement(basicXMl()));
  echo "Basic XML has " . iterator_count($i) . " root children\n";
}

<<__EntryPoint>> function main(): void {
  include __DIR__."/builtin_extensions.inc";
  test("SimpleXMLElement", "<?xml version='1.0'?><document></document>");

  testIteration();

  testCanBeWrappedByIteratorIterator();
}

<?hh
<<__EntryPoint>> function main(): void {
$xml =<<<EOF
<people>
test
  <person name="Joe"/>
  <person name="John">
    <children>
      <person name="Joe"/>
    </children>
  </person>
  <person name="Jane"/>
</people>
EOF;

$foo = simplexml_load_string( "<foo />" );
$people = simplexml_load_string($xml);

var_dump((bool)$foo);
var_dump((bool)$people);
var_dump((int)$foo);
var_dump((int)$people);
var_dump((float)$foo);
var_dump((float)$people);
var_dump($foo->__toString());
var_dump($people->__toString());

echo "===DONE===\n";
}

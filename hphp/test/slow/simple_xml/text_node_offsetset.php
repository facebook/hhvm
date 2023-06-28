<?hh


<<__EntryPoint>>
function main_text_node_offsetset() :mixed{
$xml = <<<XML
<test>
  <tag>text</tag>
</test>
XML;

$test = simplexml_load_string($xml);

$test->tag->offsetSet('foo', 'bar');
var_dump($test->tag->offsetGet('foo'));
}

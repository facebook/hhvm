<?hh <<__EntryPoint>> function main(): void {
$xml = '<node xmlns:pre="http://foo.com/tr/pre"
              xmlns:post="http://foo.com/tr/post"
              pre:type="bar" type="foo" ><sub /></node>';
$dom = new DomDocument();
$dom->loadXML($xml);
echo $dom->firstChild->getAttribute('type')."\n";
echo $dom->firstChild->getAttribute('pre:type')."\n";

$dom->firstChild->setAttribute('pre:type', 'bar2');
$dom->firstChild->setAttribute('type', 'foo2');
$dom->firstChild->setAttribute('post:type', 'baz');
$dom->firstChild->setAttribute('new:type', 'baz2');

echo $dom->firstChild->getAttribute('type')."\n";
echo $dom->firstChild->getAttribute('pre:type')."\n";
echo $dom->firstChild->getAttribute('post:type')."\n";

$dom->firstChild->removeAttribute('pre:type');
$dom->firstChild->removeAttribute('type');

echo $dom->firstChild->getAttribute('type')."\n";
echo $dom->firstChild->getAttribute('pre:type')."\n";
echo $dom->firstChild->getAttribute('post:type')."\n";
echo $dom->firstChild->getAttribute('new:type');
}

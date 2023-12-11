<?hh

<<__EntryPoint>>
function main_canonicalization() :mixed{
$xml = <<<EOXML
<?xml version="1.0" encoding="ISO-8859-1" ?>
<foo xmlns="http://www.example.com/ns/foo"
     xmlns:fubar="http://www.example.com/ns/fubar" xmlns:test="urn::test"><contain>
  <bar><test1 /></bar>
  <bar><test2 /></bar>
  <fubar:bar xmlns:fubar="http://www.example.com/ns/fubar"><test3 /></fubar:bar>
  <fubar:bar><test4 /></fubar:bar>
<!-- this is a comment -->
</contain>
</foo>
EOXML;

$dom = new DOMDocument();
$dom->loadXML($xml);
$doc = $dom->documentElement->firstChild;

/* inclusive/without comments first child element of doc element is context. */
echo $doc->C14N()."\n\n";

/* exclusive/without comments first child element of doc element is context. */
echo $doc->C14N(TRUE)."\n\n";

/* inclusive/with comments first child element of doc element is context. */
echo $doc->C14N(FALSE, TRUE)."\n\n";

/* exclusive/with comments first child element of doc element is context. */
echo $doc->C14N(TRUE, TRUE)."\n\n";

/* exclusive/without comments using xpath query. */
echo $doc->C14N(TRUE, FALSE, dict['query'=>'(//. | //@* | //namespace::*)'])."\n\n";

/* exclusive/without comments first child element of doc element is context.
   using xpath query with registered namespace.
   test namespace prefix is also included. */
echo $doc->C14N(TRUE, FALSE,
                dict[
                  'query'=>'(//a:contain | //a:bar | .//namespace::*)',
                  'namespaces'=>dict['a'=>'http://www.example.com/ns/foo']
                ],
                vec['test'])."\n\n";

/* exclusive/without comments first child element of doc element is context.
   test namespace prefix is also included */
echo $doc->C14N(TRUE, FALSE, NULL, vec['test']);
}

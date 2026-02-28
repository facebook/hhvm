<?hh

// Test from https://bugs.php.net/bug.php?id=49490
<<__EntryPoint>>
function main_xpath_registerns() :mixed{
$dom = new DOMDocument();
$dom->loadXML(
  '<foobar><a:foo xmlns:a="urn:a">'.
  '<b:bar xmlns:b="urn:b"/></a:foo>'.
  '</foobar>'
);
$xpath = new DOMXPath($dom);

//get context node and check "a:foo"
$context = $dom->documentElement->firstChild;
var_dump($context->tagName);

// try to override the context node
$xpath->registerNamespace('a', 'urn:b');
var_dump(
  $xpath->evaluate(
    'descendant-or-self::a:*',
    $context,
    false
  )->item(0)->tagName
);

// use a prefix not used in context
$xpath->registerNamespace('prefix', 'urn:b');
var_dump(
  $xpath->evaluate(
    'descendant-or-self::prefix:*',
    $context,
    false
  )->item(0)->tagName
);
}

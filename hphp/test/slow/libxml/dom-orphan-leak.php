<?hh
// c.f. https://github.com/facebook/hhvm/issues/4086

function foo() {
  for ($i = 0; $i < 1000; $i++) {
    $doc = new DOMDocument();
    $doc->loadHTML('<html><body></body></html>');
    $el = $doc->getElementsByTagName('body')->item(0);
    $el->parentNode->removeChild($el);
  }
}

function test() {
  foo(); foo(); foo();
  $start = memory_get_peak_usage(true);
  foo();
  $end = memory_get_peak_usage(true);
  return $end - $start;
}


<<__EntryPoint>>
function main_dom_orphan_leak() {
$x = test();
$x = test();
echo $x, "\n";
}

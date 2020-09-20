<?hh
// c.f. https://github.com/facebook/hhvm/issues/4084

function foo() {
  for ($i=0; $i<100; $i++) {
    new DOMText('hi');
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
function main_dom_text_leak() {
$x = test();
$x = test();
echo $x, "\n";
}

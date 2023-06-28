<?hh


<<__EntryPoint>>
function main_clone() :mixed{
$dom = new DOMDocument();
$dom->loadHTML('
<html>
  <body>
    <div id="foo">bar</div>
  </body>
</html>');
$clone = clone $dom;
$body = $clone->getElementsByTagName('body')->item(0);
$body->appendChild(new DOMElement('asdf'));
var_dump($clone->saveHTML());
}

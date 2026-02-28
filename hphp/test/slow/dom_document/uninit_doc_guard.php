<?hh

class MyDoc extends DOMDocument {
  function __construct($version, $encoding)[] {}
}

class MyOtherDoc extends DOMDocument {
  function __construct($version, $encoding) {
    parent::__construct($version, $encoding);
  }
}


<<__EntryPoint>>
function main_uninit_doc_guard() :mixed{
$doc = new MyDoc('1', "");
$doc->normalizeDocument(); // warning
var_dump($doc->recover);
$doc->recover = 12;

$doc = new MyOtherDoc('1', "");
$doc->normalizeDocument();
var_dump($doc->recover);
$doc->recover = 12;
echo "ok\n";
}

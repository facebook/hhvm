<?hh

<<__EntryPoint>>
function main() :mixed{
  require_once('dom_test.inc');

  $dom = new DOMDocument;
  $dom->loadXML(getXmlStr());

  if( !$dom )
  {
      echo "Error while parsing the document\n";
      exit;
  }

  $temp_str_14883 = (string)($dom->encoding);

  echo "Empty Encoding Read: {$temp_str_14883}\n";

  $ret = 'NYPHP DOMinatrix'; $dom->encoding = $ret;
  echo "Adding invalid encoding: $ret\n";

  $ret = 'ISO-8859-1'; $dom->encoding = $ret;
  echo "Adding ISO-8859-1 encoding: $ret\n";
  echo "ISO-8859-1 Encoding Read: {$dom->encoding}\n";

  $ret = 'UTF-8'; $dom->encoding = $ret;
  echo "Adding UTF-8 encoding: $ret\n";
  echo "UTF-8 Encoding Read: {$dom->encoding}\n";

  $ret = 'UTF-16'; $dom->encoding = $ret;
  echo "Adding UTF-16 encoding: $ret\n";
  echo "UTF-16 Encoding Read: {$dom->encoding}\n";
}

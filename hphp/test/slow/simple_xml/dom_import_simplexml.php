<?hh

function main() :mixed{
  $s = simplexml_load_string('<a><b>c</b></a>');
  $dom = dom_import_simplexml($s);
  print $dom->ownerDocument->saveXML();
}

<<__EntryPoint>>
function main_dom_import_simplexml() :mixed{
main();
}

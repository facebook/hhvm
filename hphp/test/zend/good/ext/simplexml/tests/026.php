<?hh

function traverse_xml($xml, $pad = '')
:mixed{
  $name = $xml->getName();
  echo "$pad<$name";
  foreach($xml->attributes() as $attr => $value)
  {
    echo " $attr=\"$value\"";
  }
  echo ">" . trim((string)$xml) . "\n";
  foreach($xml->children() as $node)
  {
    traverse_xml($node, $pad.'  ');
  }
  echo $pad."</$name>\n";
}
<<__EntryPoint>>
function entrypoint_026(): void {
  $xml =<<<EOF
<people>
  <person>Jane</person>
</people>
EOF;


  $people = simplexml_load_string($xml);
  traverse_xml($people);

  echo "===DONE===\n";
}

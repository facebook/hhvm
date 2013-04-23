<?php 
$xml =<<<EOF
<people></people>
EOF;

function traverse_xml($xml, $pad = '')
{
  $name = $xml->getName();
  echo "$pad<$name";
  foreach($xml->attributes() as $attr => $value)
  {
    echo " $attr=\"$value\"";
  }
  echo ">" . trim($xml) . "\n";
  foreach($xml->children() as $node)
  {
    traverse_xml($node, $pad.'  ');
  }
  echo $pad."</$name>\n";
}


$people = simplexml_load_string($xml);
traverse_xml($people);
$people->person = 'Joe';
$people->person['gender'] = 'male';
traverse_xml($people);
$people->person = 'Jane';
traverse_xml($people);
$people->person['gender'] = 'female';
$people->person[1] = 'Joe';
$people->person[1]['gender'] = 'male';
traverse_xml($people);
$people->person[3] = 'Minni-me';
$people->person[2]['gender'] = 'male';
traverse_xml($people);
$people->person[3]['gender'] = 'error';
traverse_xml($people);
?>
===DONE===
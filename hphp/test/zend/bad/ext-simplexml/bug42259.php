<?php
$xml =<<<EOF
<xml>
  <fieldset1>
    <field1/>
    <field2/>
  </fieldset1>
  <fieldset2>
    <options>
      <option1/>
      <option2/>
      <option3/>
    </options>
    <field1/>
    <field2/>
  </fieldset2>
</xml>
EOF;

$sxe = new SimpleXMLIterator($xml);
$rit = new RecursiveIteratorIterator($sxe, RecursiveIteratorIterator::LEAVES_ONLY);
foreach ($rit as $child) {
  $path = '';
  $ancestry = $child->xpath('ancestor-or-self::*');
  foreach ($ancestry as $ancestor) {
    $path .= $ancestor->getName() . '/';
  }
  $path = substr($path, 0, strlen($path) - 1);
  echo count($ancestry) . ' steps: ' . $path . PHP_EOL;
}
?>
===DONE===
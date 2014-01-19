<?php

function convert_simplexml_to_array($sxml) {
  $arr = array();
  if ($sxml) {
    foreach ($sxml as $k => $v) {
      if ($sxml['list']) {
        if (isset($v['key'])) {
          $arr[(string)$v['key']] = convert_simplexml_to_array($v);
        }
 else {
          $arr[] = convert_simplexml_to_array($v);
        }
      }
 else {
        $arr[$k] = convert_simplexml_to_array($v);
      }
    }
  }
  if (sizeof($arr) > 0) {
    return $arr;
  }
 else {
    return (string)$sxml;
  }
}

$xml = <<<EOM
<root list="true">
  <node key="key1" list="true">
    <subnode key="subkey1" list="true">
      <name>value1-1</name>
      <name>value1-2</name>
    </subnode>
    <subnode key="subkey2">value1</subnode>
  </node>
  <node key="key2" list="true">
    <subnode>value2</subnode>
  </node>
</root>
EOM;

$sxml = simplexml_load_string($xml);
var_dump(convert_simplexml_to_array($sxml));

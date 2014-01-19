<?php

$file = <<<EOM
<?xml version="1.0" encoding="UTF-8"?>
<wurfl-config>
  <persistence>
    <provider>memcache</provider>
    <params></params>
  </persistence>

  <cache>
    <provider>memcache</provider>
    <params></params>
  </cache>
</wurfl-config>
EOM;
var_dump($file);

$xml = simplexml_load_string($file);
foreach ($xml->children() as $parent_name => $xml_ele) {
  var_dump($parent_name);

  foreach ($xml_ele->children() as $key => $value) {
    var_dump((string)$key, (string)$value);
  }
}

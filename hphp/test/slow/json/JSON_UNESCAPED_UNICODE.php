<?php

$tests = array(
  html_entity_decode('&#xd800;&#xdc00;', ENT_NOQUOTES, 'utf-8'),
  htmlspecialchars_decode('&#x1D11E;')
);
foreach ($tests as $s) {
  var_dump($s);
  var_dump(json_encode($s));
  var_dump(json_encode($s, JSON_UNESCAPED_UNICODE));
}

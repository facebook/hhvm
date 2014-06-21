<?php

$test = [
  '&#34;&#x32;',
  '&#9829;&#x2665;',
];

foreach ($test as $str) {
  var_dump(bin2hex(htmlspecialchars_decode($str)));
  var_dump(bin2hex(html_entity_decode($str)));
}

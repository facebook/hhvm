<?php

function t($xml) {
  $dom = new DOMDocument();
  $dom->loadHTML($xml);
  $child = $dom->getElementById('x');
  $child = null;
  $child = $dom->getElementById('x');
}

t('<html><body><div id="x"></div></body></html>');
t('<html><div id="x"/></html>');
t('<html><form id="x" action="" method="POST"></html>');

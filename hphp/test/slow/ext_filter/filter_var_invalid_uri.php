<?php

$urls = array(
  "http://www.facebook.com/valid",
  "http://www.facebook.com/with space",
  "http://www.facebook.com/with\ttab",
  "http://www.facebook.com/with\nnewline",
  "http://www.facebook.com/with\0endline",
);

var_dump(array_map(function($url) {
  return filter_var($url, FILTER_VALIDATE_URL);
}, $urls));

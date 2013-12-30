<?php
$data = [
  '"foo\nbar"',
  '"baz\u003dbong"',
];

foreach($data as $str) {
  var_dump(json_decode($str));
}

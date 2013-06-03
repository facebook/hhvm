<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function r($a) {
  var_dump(is_resource($a));
  var_dump(is_object($a));
  var_dump($a);
}

$f = imagecreate(10, 10);
r($f);
imagedestroy($f);
// Now it's not a resource anymore! It's not an object either. What is it?
// It's PHP, that's what.
r($f);

<?php
ini_set('memory_limit', '100K');
for ($i = 0; $i < 100000; $i++) {
  $x = json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE);
}
var_dump($x);

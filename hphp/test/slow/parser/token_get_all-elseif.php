<?php

$code = '<?php
if ($a == 1) {
} else if ($a == 2) {
} eLSe     iF ($a == 3) {
} else
if ($a == 4) {
} elseif ($a == 5) {
}';

foreach (token_get_all($code) as $token) {
  if (is_array($token)) {
    $token[0] = token_name($token[0]);
  }
  var_dump($token);
}

<?php

function f($code) {
  $tokens = token_get_all($code);
  foreach($tokens as $token) {
    $t = is_string($token) ? $token : $token[1];
    var_dump(addcslashes($t, "\r\n"));
  }
}


<<__EntryPoint>>
function main_single_line_comment_with_mac_newline() {
f("<?php // \r");
f("<?php // \r\r");
}

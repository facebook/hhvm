<?php

function f($code) {
  $tokens = token_get_all($code);
  foreach($tokens as $token) {
    $t = is_string($token) ? $token : $token[1];
    var_dump(addcslashes($t, "\r\n"));
  }
}


<<__EntryPoint>>
function main_single_line_comment_with_eof() {
f("<?hh // ?>");
f("<?php // ?>");
f("<?php // comment");
}

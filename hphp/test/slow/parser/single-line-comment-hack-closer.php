<?php

function f($code) {
  $tokens = token_get_all($code);
  foreach($tokens as $token) {
    $id = is_string($token) ? "'$token'" : token_name($token[0]);
    $t = is_string($token) ? $token : $token[1];
    echo "{$id} => ";
    var_dump(addcslashes($t, "\r\n"));
  }
}


<<__EntryPoint>>
function main_single_line_comment_hack_closer() {
f("<?hh // ?>\n");
f("<?hh // ?>");
}

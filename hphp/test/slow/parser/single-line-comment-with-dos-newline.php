<?php

function f($code) {
  $tokens = token_get_all($code);
  foreach($tokens as $token) {
    $t = is_string($token) ? $token : $token[1];
    var_dump(addcslashes($t, "\r\n"));
  }
}


// Repeat off-by-one because the old lexer worked right
// with the second version, but not the first, so test both.
<<__EntryPoint>>
function main_single_line_comment_with_dos_newline() {
f("<?php // \r\n");
f("<?php //  \r\n");
}

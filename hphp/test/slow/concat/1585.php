<?php

function n_() {
  return "
" .  str_repeat($GLOBALS['n_indent_tab'], $GLOBALS['n_indent_level']);
}
function n_indent() {
  $GLOBALS['n_indent_level']++;
  return n_();
}
function n_unindent() {
  $GLOBALS['n_indent_level']--;
  return n_();
}
function render($arg1, $arg2) {
    return      '<div id="captcha" class="'.$arg1.'">'.      n_indent().      $arg2 .      n_unindent().      '</div>';
}
$GLOBALS['n_indent_level'] = 0;
var_dump(render("foo", "bar"));

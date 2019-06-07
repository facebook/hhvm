<?hh

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

<<__EntryPoint>>
function main_1585() {
$GLOBALS['n_indent_level'] = 0;
$GLOBALS['n_indent_tab'] = "\t";
var_dump(render("foo", "bar"));
}

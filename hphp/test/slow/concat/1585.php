<?hh

function n_() {
  return "
" .  str_repeat(\HH\global_get('n_indent_tab'), \HH\global_get('n_indent_level'));
}
function n_indent() {
  \HH\global_set('n_indent_level', \HH\global_get('n_indent_level') + 1);
  return n_();
}
function n_unindent() {
  \HH\global_set('n_indent_level', \HH\global_get('n_indent_level') - 1);
  return n_();
}
function render($arg1, $arg2) {
    return      '<div id="captcha" class="'.$arg1.'">'.      n_indent().      $arg2 .      n_unindent().      '</div>';
}

<<__EntryPoint>>
function main_1585() {
\HH\global_set('n_indent_level', 0);
\HH\global_set('n_indent_tab', "\t");
var_dump(render("foo", "bar"));
}

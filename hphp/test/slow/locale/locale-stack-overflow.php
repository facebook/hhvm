<?hh

function test($len) {
  return locale_get_display_name(str_repeat('*', $len), 'a');
}


<<__EntryPoint>>
function main_locale_stack_overflow() {
test(254);
test(255);
test(256);
test(257);
}

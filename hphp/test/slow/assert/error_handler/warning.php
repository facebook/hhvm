<?hh


<<__EntryPoint>>
function main_warning() {
assert_options(ASSERT_WARNING, 1);
assert_options(ASSERT_BAIL, 0);
require_once __DIR__.'/main.inc';
test();
}

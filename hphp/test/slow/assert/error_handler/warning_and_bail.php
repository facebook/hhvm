<?php


<<__EntryPoint>>
function main_warning_and_bail() {
assert_options(ASSERT_WARNING, 1);
assert_options(ASSERT_BAIL, 1);
require_once __DIR__.'/main.inc';
}

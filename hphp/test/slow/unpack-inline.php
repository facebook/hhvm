<?hh

<<__ALWAYS_INLINE>>
function varargs(int $one, bool $two, ...$three) {
  return $three[2];
}

<<__ALWAYS_INLINE>>
function mix(...$args) {
  return varargs(12, true, ...$args);
}

<<__NEVER_INLINE>>
function bar() {
  echo mix('alpha', new stdClass, 'gamma', varray[])."\n";
}

<<__EntryPoint>>
function main() {
  bar(); bar(); bar(); bar();
}

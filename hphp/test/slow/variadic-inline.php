<?hh

<<__ALWAYS_INLINE>>
function varargs(int $one, bool $two, ...$three) {
  return $three[2];
}

<<__NEVER_INLINE>>
function bar() {
  echo varargs(2, false, 'alpha', 'beta', 'gamma', 'delta')."\n";
}

<<__EntryPoint>>
function main() {
  bar(); bar(); bar(); bar();
}

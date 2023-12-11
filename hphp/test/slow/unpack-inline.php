<?hh

<<__ALWAYS_INLINE>>
function varargs(int $one, bool $two, ...$three) :mixed{
  return $three[2];
}

<<__ALWAYS_INLINE>>
function mix(...$args) :mixed{
  return varargs(12, true, ...$args);
}

<<__NEVER_INLINE>>
function bar() :mixed{
  echo mix('alpha', new stdClass, 'gamma', vec[])."\n";
}

<<__EntryPoint>>
function main() :mixed{
  bar(); bar(); bar(); bar();
}

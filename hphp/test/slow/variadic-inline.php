<?hh

<<__ALWAYS_INLINE>>
function varargs(int $one, bool $two, ...$three) :mixed{
  return $three[2];
}

<<__NEVER_INLINE>>
function bar() :mixed{
  echo varargs(2, false, 'alpha', new stdClass, 'gamma', new Exception)."\n";
}

<<__EntryPoint>>
function main() :mixed{
  bar(); bar(); bar(); bar();
}

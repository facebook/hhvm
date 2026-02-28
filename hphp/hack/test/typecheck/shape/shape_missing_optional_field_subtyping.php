<?hh

type s = shape(...);
type t = shape(?'z' => mixed, ...);

function test(s $s): t {
  return $s;
}

<?hh

// Non-async function returning Awaitable should NOT unwrap.
// The return type is Awaitable<int> itself, not int.
function foo(): Awaitable<int> {
  $x = async { return 42; };
  return $x;
//       ^ enforcement-at-caret
}

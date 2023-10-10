<?hh

final class :my-xhp {
  attribute int x;
}

<<__EntryPoint>>
async function f(): Awaitable<void> {
  <my-xhp x={await async { return 42; }}>
    {await async { return null; }}
  </my-xhp>;
  print "good";
}

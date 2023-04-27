<?hh

<<__Memoize>>
function memo() {
  var_dump("memo");
}

async function baz() {
  await RescheduleWaitHandle::create(0, 0);
  var_dump("baz");
  memo();
}

async function bar() {
  await RescheduleWaitHandle::create(0, 0);
  var_dump("bar");
  $wh = baz();
  register_postsend_function(() ==> {
    var_dump("in psp");
    HH\Asio\join($wh);
    var_dump("after join");
    memo();
    var_dump("done with psp");
  });
  __hhvm_intrinsics\trigger_oom(true);
}

<<__EntryPoint>>
async function main() {
  await HH\ImplicitContext\soft_run_with(
    bar<>,
    'abc',
  );
  var_dump("done");
}

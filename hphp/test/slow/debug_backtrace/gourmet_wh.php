<?hh

// Usually, me and Vince would be happy with some freeze-dried Taster's Choice.

class G {
  async function doit() :Awaitable<mixed>{
    await RescheduleWaitHandle::create(0, 3);
    return 1;
  }
}

<<__EntryPoint>>
async function gourmet(): Awaitable<void> {
  Exception::setTraceOptions(DEBUG_BACKTRACE_PROVIDE_OBJECT);
  try {
    await A();
  } catch(Exception $e) {
    echo "Thrown!\n";
  }
}

async function A(): Awaitable<void> {
  $v = new Vector();
  $v[] = B($v);
  await RescheduleWaitHandle::create(0, 1);
  await $v[0];
}

async function B($v): Awaitable<void> {
  await C($v);
}

function obtainWait() :mixed{
  return new G()->doit();
}

async function C($v): Awaitable<void> {
  await RescheduleWaitHandle::create(0, 2);
  $abl = obtainWait();
  await AwaitAllWaitHandle::fromVec(vec[
    $abl,
    $v[0]
  ]);
}

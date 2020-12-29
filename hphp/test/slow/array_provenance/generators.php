<?hh

class C {}

<<__ProvenanceSkipFrame>>
function iter($xs) {
  yield darray['result' => new C()];
  foreach ($xs as $x) {
    yield darray['result' => new C()];
  }
  yield darray['result' => new C()];
}

<<__ProvenanceSkipFrame>>
async function async_iter($xs) {
  yield darray['result' => new C()];
  foreach ($xs as $x) {
    await $x;
    yield darray['result' => new C()];
  }
  yield darray['result' => new C()];
}

<<__EntryPoint>>
async function main() {
  $xs = vec[new C(), new C()];
  foreach (iter($xs) as $k => $v) {
    print(HH\get_provenance($v)."\n");
  }

  $xs = vec[
    RescheduleWaitHandle::create(1, 1),
    RescheduleWaitHandle::create(1, 1),
  ];
  foreach (async_iter($xs) await as $k => $v) {
    print(HH\get_provenance($v)."\n");
  }
}

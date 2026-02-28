<?hh

class Ref {
  function __construct(public $value)[] {}
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  $join_it = new Ref(0);

  ExternalThreadEventWaitHandle::setOnCreateCallback($ete ==> {
    echo "onCreateCallback()...\n";
    if ($join_it->value === 1) exit(0);
  });

  await __hhvm_intrinsics\dummy_dict_await();
  ++$join_it->value;
  await __hhvm_intrinsics\dummy_dict_await();
}

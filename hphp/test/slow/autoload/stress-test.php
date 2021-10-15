<?hh

<<__DynamicallyCallable>>
function worker(): void {
  HH\autoload_set_paths(
    dict[
      'class' => dict[
        'clsroot' => 'stress-test-1.inc',
        'clsparent' => 'stress-test-2.inc',
        'clschild' => 'stress-test-2.inc',
        'x' => 'stress-test-2.inc',
      ]
    ],
    __DIR__.'/',
  );

  new X();
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  if (HH\execution_context() === "xbox") return;

  $v = vec[];
  for ($i = 0; $i < 10; ++$i) {
    $v[] = fb_gen_user_func_array(
      __FILE__,
      'worker',
      vec[],
    );
  }
  await AwaitAllWaitHandle::fromVec($v);
  echo "done\n";
}


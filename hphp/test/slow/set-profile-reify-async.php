<?hh

class C {}

async function test<reify T>() :Awaitable<mixed>{
  $x = dict[17 => new C()];
  await RescheduleWaitHandle::create(1, 1);
  print(json_encode($x[17])."\n");
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  fb_setprofile(
    ($mode, $fn, $frame) ==> {
      $classes = $frame['reified_classes'] ?? null;
      if ($classes !== null) {
        print("$mode: ".json_encode($classes)."\n");
      }
    },
    SETPROFILE_FLAGS_ENTERS|SETPROFILE_FLAGS_RESUME_AWARE,
  );
  await test<C>();
}

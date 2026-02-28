<?hh

class C {
  public static int $x = 0;
}

function handler($what, $fun) :mixed{
  if ($fun == 'foo' && $what == 'exit') {
    if (C::$x == 1) {
      exit(1);
    }
    C::$x = 1;
  }
}

async function foo() :AsyncGenerator<mixed,mixed,void>{
  echo "enter\n";
  $x = RescheduleWaitHandle::create(0, 0);
  await $x;
  yield null;
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  fb_setprofile(handler<>);
  foreach (foo() await as $_) {}
  echo "NOOOOO!\n";
}

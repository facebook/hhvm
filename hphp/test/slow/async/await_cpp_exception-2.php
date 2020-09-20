<?hh

class C {
  public static int $x = 0;
}

function handler($what, $fun) {
  if ($fun == 'foo' && $what == 'exit') {
    if (C::$x == 1) {
      exit(1);
    }
    C::$x = 1;
  }
}

async function foo() {
  echo "enter\n";
  $x = RescheduleWaitHandle::create(0, 0);
  await $x;
  yield null;
}

<<__EntryPoint>>
async function main() {
  fb_setprofile('handler');
  foreach (foo() await as $_) {}
  echo "NOOOOO!\n";
}

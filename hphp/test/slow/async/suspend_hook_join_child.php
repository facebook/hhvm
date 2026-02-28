<?hh

<<__EntryPoint>>
async function main(): Awaitable<void> {
  // Suspend main() into a new Awaitable.
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);

  $child = RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 0);
  fb_setprofile(($why, $what) ==> {
    if ($why == 'exit' && $what == 'main') {
      echo "suspending or returning from main\n";
      HH\Asio\join($child);
    }
  });

  // Determine
  // Create dependency between main and child.
  await $child;
  echo "done\n";
}

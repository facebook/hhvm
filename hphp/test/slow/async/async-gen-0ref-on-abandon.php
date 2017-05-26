<?hh

async function foo($outer_reschedule) {
  if (false) { yield 42; }
  await $outer_reschedule;
}

$outer_reschedule = RescheduleWaitHandle::create(0, 0);
foo($outer_reschedule)->next();
\HH\Asio\join($outer_reschedule);
echo "survived\n";

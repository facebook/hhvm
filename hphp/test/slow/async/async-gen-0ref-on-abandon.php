<?hh

async function foo($outer_reschedule) :AsyncGenerator<mixed,mixed,void>{
  if (false) { yield 42; }
  await $outer_reschedule;
}


<<__EntryPoint>>
function main_async_gen_0ref_on_abandon() :mixed{
$outer_reschedule = RescheduleWaitHandle::create(0, 0);
foo($outer_reschedule)->next();
\HH\Asio\join($outer_reschedule);
echo "survived\n";
}

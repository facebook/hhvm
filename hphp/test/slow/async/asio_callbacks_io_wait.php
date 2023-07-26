<?hh


<<__EntryPoint>>
function main_asio_callbacks_io_wait() :mixed{
Awaitable::setOnIOWaitEnterCallback(function() {
  echo "io wait enter\n";
});

Awaitable::setOnIOWaitExitCallback(function(?WaitableWaitHandle $wh) {
  echo "io wait exit\n";
  invariant($wh !== null, 'not null');
  var_dump(HH\Asio\backtrace($wh));
});

echo "going to sleep\n";
HH\Asio\join(SleepWaitHandle::create(2000000));
echo "sleep finished\n";
}

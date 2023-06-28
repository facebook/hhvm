<?hh


<<__EntryPoint>>
function main_asio_callbacks_io_wait() :mixed{
Awaitable::setOnIOWaitEnterCallback(function() {
  echo "io wait enter\n";
});

Awaitable::setOnIOWaitExitCallback(function() {
  echo "io wait exit\n";
});

echo "going to sleep\n";
HH\Asio\join(SleepWaitHandle::create(2000000));
echo "sleep finished\n";
}

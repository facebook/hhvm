<?hh


<<__EntryPoint>>
function main_asio_callbacks_sleep() :mixed{
SleepWaitHandle::setOnCreateCallback(function($wait_handle) {
  echo get_class($wait_handle)." enter\n";
});

SleepWaitHandle::setOnSuccessCallback(function($wait_handle) {
  echo get_class($wait_handle)." exit\n";
});

echo "going to sleep\n";
HH\Asio\join(SleepWaitHandle::create(1));
echo "sleep finished\n";
}

<?hh

SleepWaitHandle::setOnCreateCallback(function($wait_handle) {
  echo get_class($wait_handle)." enter\n";
});

SleepWaitHandle::setOnSuccessCallback(function($wait_handle) {
  echo get_class($wait_handle)." exit\n";
});

echo "going to sleep\n";
SleepWaitHandle::create(1)->join();
echo "sleep finished\n";

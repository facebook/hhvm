<?hh

WaitableWaitHandle::setOnIOWaitEnterCallback(function() {
  echo "io wait enter\n";
});

WaitableWaitHandle::setOnIOWaitExitCallback(function() {
  echo "io wait exit\n";
});

echo "going to sleep\n";
HH\Asio\join(SleepWaitHandle::create(2000000));
echo "sleep finished\n";

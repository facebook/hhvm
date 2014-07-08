<?hh

WaitHandle::setOnIOWaitEnterCallback(function() {
  echo "io wait enter\n";
});

WaitHandle::setOnIOWaitExitCallback(function() {
  echo "io wait exit\n";
});

echo "going to sleep\n";
SleepWaitHandle::create(1000000)->join();
echo "sleep finished\n";

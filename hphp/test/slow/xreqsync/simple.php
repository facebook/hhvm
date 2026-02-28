<?hh

<<__EntryPoint>>
function main(): void {
  $lockname = 'lock'.time().rand();
  $lock = HH\XReqSync::get($lockname);

  echo "Getting lock...\n";
  var_dump(HH\Asio\join($lock->genLock()));

  echo "Getting lock again multiple times...\n";
  $lock2 = HH\XReqSync::get($lockname);
  var_dump(HH\Asio\join($lock->genLock()));
  var_dump(HH\Asio\join($lock2->genLock()));

  echo "Releasing lock multiple times...\n";
  $lock->unlock();
  $lock->unlock();
  $lock->unlock();
  $lock2->unlock();
}

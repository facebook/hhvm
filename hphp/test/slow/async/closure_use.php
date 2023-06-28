<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
<<__EntryPoint>>
function main_closure_use() :mixed{
;

// closure in use param

$callback = async function() { return 42; };
$x = async function() use ($callback) {
  return await $callback();
};
$y = async function() use ($callback) {
  await block();
  return await $callback();
};

var_dump(HH\Asio\join($x($callback)));
var_dump(HH\Asio\join($y($callback)));

// reference passing

$env = 3;

$inc = async function () use ($env) { return ++$env; };
$incB = async function () use ($env) { await block(); return ++$env; };

var_dump(HH\Asio\join($inc()));
var_dump(HH\Asio\join($incB()));
var_dump($env);
}

<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

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
$incref = async function () use (&$env) { return ++$env; };
$increfB = async function () use (&$env) { await block(); return ++$env; };

var_dump(HH\Asio\join($inc()));
var_dump(HH\Asio\join($incB()));
var_dump($env);

var_dump(HH\Asio\join($incref()));
var_dump($env);

var_dump(HH\Asio\join($increfB()));
var_dump($env);

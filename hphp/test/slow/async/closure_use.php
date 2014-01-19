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

var_dump($x($callback)->join());
var_dump($y($callback)->join());

// reference passing

$env = 3;

$inc = async function () use ($env) { return ++$env; };
$incB = async function () use ($env) { await block(); return ++$env; };
$incref = async function () use (&$env) { return ++$env; };
$increfB = async function () use (&$env) { await block(); return ++$env; };

var_dump($inc()->join());
var_dump($incB()->join());
var_dump($env);

var_dump($incref()->join());
var_dump($env);

var_dump($increfB()->join());
var_dump($env);

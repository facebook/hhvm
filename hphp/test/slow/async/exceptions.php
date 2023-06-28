<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function aThrow() :Awaitable<mixed>{ throw new Exception(__FUNCTION__); }
async function aaThrow() :Awaitable<mixed>{ await aThrow(); }

async function bThrow() :Awaitable<mixed>{ await block(); throw new Exception(__FUNCTION__); }
async function bbThrow() :Awaitable<mixed>{ await block(); await bThrow(); }

function verify($a, $e) :mixed{
  try { \HH\Asio\result($a); }
  catch (Exception $ae) {}
  invariant($ae->getMessage() == $e->getMessage(), "");
  var_dump($e->getMessage());
}

function normalCatch() :mixed{
  try { $a = aThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = aaThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bbThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }
}

async function asyncCatch() :Awaitable<mixed>{
  try { $a = aThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = aaThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bbThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }
}


<<__EntryPoint>>
function main_exceptions() :mixed{

normalCatch();
echo "---\n";
HH\Asio\join(asyncCatch());
}

<?hh

function block() {
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function aThrow() { throw new Exception(__FUNCTION__); }
async function aaThrow() { await aThrow(); }

async function bThrow() { await block(); throw new Exception(__FUNCTION__); }
async function bbThrow() { await block(); await bThrow(); }

function verify($a, $e) {
  try { \HH\Asio\result($a); }
  catch (Exception $ae) {}
  invariant($ae->getMessage() == $e->getMessage(), "");
  var_dump($e->getMessage());
}

function normalCatch() {
  try { $a = aThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = aaThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bbThrow(); HH\Asio\join($a); }
  catch (Exception $e) { verify($a, $e); }
}

async function asyncCatch() {
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
function main_exceptions() {

normalCatch();
echo "---\n";
HH\Asio\join(asyncCatch());
}

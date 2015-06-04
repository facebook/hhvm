<?hh
assert_options(ASSERT_ACTIVE, 1);
assert_options(ASSERT_WARNING, 1);

function block() { return RescheduleWaitHandle::create(1,1); };

async function aThrow() { throw new Exception(__function__); }
async function aaThrow() { await aThrow(); }

async function bThrow() { await block(); throw new Exception(__function__); }
async function bbThrow() { await block(); await bThrow(); }

function verify(&$a, &$e) {
  try { $a->result(); }
  catch (Exception $ae) {}
  assert($ae->getMessage() == $e->getMessage());
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

normalCatch();
echo "---\n";
HH\Asio\join(asyncCatch());


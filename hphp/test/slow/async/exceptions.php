<?hh
assert_options(ASSERT_ACTIVE, 1);
assert_options(ASSERT_WARNING, 1);

function block() { return RescheduleWaitHandle::create(1,1); };

async function aThrow() { throw new Exception(__function__); }
async function aaThrow() { await aThrow(); }

async function bThrow() { await block(); throw new Exception(__function__); }
async function bbThrow() { await block(); await bThrow(); }

function verify(&$a, &$e) {
  assert($a->getExceptionIfFailed()->getMessage() == $e->getMessage());
  var_dump($e->getMessage());
}

function normalCatch() {
  try { $a = aThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = aaThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bbThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }
}

async function asyncCatch() {
  try { $a = aThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = aaThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }

  try { $a = bbThrow(); $a->join(); }
  catch (Exception $e) { verify($a, $e); }
}

normalCatch();
echo "---\n";
asyncCatch()->join();


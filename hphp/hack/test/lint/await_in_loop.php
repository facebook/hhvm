<?hh

async function await_in_loop($k, $n) {
  foreach ($k as $v) {
    await $v;
  }

  $test_array = vec[];
  foreach ($k as $v) {
    $test_array[] = await $v;
  }

  while (true) {
    await $n;
  }

  if (true) {
    await $n;
  }

  while (true) {
    if (true) {
      await $n;
    }
  }
}

async function loop_in_async_block($n) {
  return await async {
    while (true) {
      await $n;
    }
  };
}

async function loop_in_async_block_in_loop($k, $n) {
  $blocks = vec[];
  foreach ($k as $v) {
    $blocks[] = async {
      while (true) {
        await $v;
      }
    };
  }

  return await Vec\gen($blocks);
}

// Negative examples below

async function async_blocks_in_loop($k, $n) {
  $blocks = vec[];
  foreach ($k as $v) {
    $blocks[] = async {
      await $v;
      await $v; // Make sure the linter doesn't trip over multiple awaits
    };
  }

  foreach ($k await as $v) {
    await $v;
  }

  return await Vec\gen($blocks);
}

// Negative examples for named functions with awaits
async function async_fun_helper($v) {
  return await $v;
}

async function named_async_fun_blocks_in_loop($k, $n) {
  $blocks = vec[];
  foreach ($k as $v) {
    $blocks[] = async_fun_helper($v);
  }

  return await Vec\gen($blocks);
}

// Negative example for a looped synchronous function call with an await
// We might want this to be a positive case?
function sync_fun_helper($v) {
  return await $v;
}

async function named_blocking_fun_in_loop($k) {
  foreach ($k as $v) {
    sync_fun_helper($v);
  }
}

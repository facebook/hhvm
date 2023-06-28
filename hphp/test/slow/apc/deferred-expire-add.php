<?hh

<<__EntryPoint>>
function main() :mixed{
  $ok = false;
  if (apc_fetch('bar', inout $ok) === false) {
    apc_add('bar', true);
    apc_add('foo', 'bar', 1);
    echo "Added. Sleeping for 2s\n";
    sleep(2);
    return;
  }
  $val = apc_fetch('foo', inout $ok);
  if ($val === 'baz') {
    echo "Got baz!\n";
    return;
  }
  if ($val !== false) {
    echo "Oops, fetch succeeded: $val\n";
    return;
  }
  if (apc_add('foo', 'baz', 60)) {
    $val = apc_fetch('foo', inout $ok);
    if ($val !== 'baz') {
      echo "Failed to add after deferred expiry ($val)\n";
      return;
    }
    echo "Re-added as baz\n";
    return;
  }
}

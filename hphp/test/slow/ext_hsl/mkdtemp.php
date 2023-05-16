<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  // Supported number of 'X's varies by platform; only exactly 6 is portable.
  // Builtin will error if an invalid pattern is given; leave enforcing
  // portability (or not) up to the HSL.
  $pattern = sys_get_temp_dir().'/ext_hsl-test-XXXXXX';
  $tmpdir = _OS\mkdtemp($pattern);
  // All of these should be true
  var_dump($tmpdir !== $pattern);
  var_dump(strpos($tmpdir, 'XXXXXX') === false);
  var_dump(is_dir($tmpdir));
  var_dump((stat($tmpdir)['mode'] & 0777) === 0700);
  var_dump(rmdir($tmpdir));

  try {
    _OS\mkdtemp('');
  } catch (_OS\ErrnoException $e) {
    if ($e->getCode() === _OS\EINVAL) {
      print("Empty string failed with EINVAL (expected)\n");
    } else {
      printf(
        "Empty string failed with %d: %s\n",
        $e->getCode(),
        $e->getMessage()
      );
    }
  }
}

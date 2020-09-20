<?hh

namespace __SystemLib {

use namespace HH\Lib\_Private\_OS;

/** Get a temporary directory, including trailing slash.
 *
 * At the start of the test:
 * - the directory will exist
 * - the directory will be empty
 *
 * Repeat calls to this function will return the same path.
 */
<<__Memoize>>
function hphp_test_tmproot(): string {
  $override = \getenv('HPHP_TEST_TMPDIR');
  if ($override is string) {
    invariant(
      \rtrim($override, '/') === $override,
      'HPHP_TEST_TMPDIR must not have a trailing slash',
    );
    return $override.'/';
  }

  $pattern = \rtrim(\sys_get_temp_dir(), '/').'/hphp-test-XXXXXX';
  return _OS\mkdtemp($pattern).'/';
}

/** Get a temporary, absolute path, which does not yet exist.
 *
 * - Returns a path that will not exist at the start of the test.
 * - All created paths have the same parent directory (`hphp_test_tmproot()`).
 * - The parent directory (`hphp_test_tmproot()`) will exist at the start of
 *   the test.
 * - You may use this path as a filename, directory name, unix socket path etc,
 *   or as a prefix.
 *
 * It is an error to specify an empty filename, as the above expectations would
 * be violated - use `hphp_test_tmproot()` instead if you want the parent
 * directory.
 */
function hphp_test_tmppath(string $filename): string {
  invariant(
    $filename !== '',
    'Specify a filename, or use hphp_test_tmproot() instead'
  );
  $root = hphp_test_tmproot();
  return $root.$filename;
}

} // namespace

<?hh

// Same as entrypoint-symlink.php but in a namespace.
// Tests that Package::createSymlinkWrapper() correctly escapes backslashes.

namespace Foo\Bar;

<<__EntryPoint>>
function cli_main() :mixed{
  echo 'HI!';
}

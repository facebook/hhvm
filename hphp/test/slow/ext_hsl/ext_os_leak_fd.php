<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  // Make sure that cleanup doesn't crash HHVM
  $_leak_this_fd = _OS\open('/dev/null', _OS\O_RDONLY, 0);
}

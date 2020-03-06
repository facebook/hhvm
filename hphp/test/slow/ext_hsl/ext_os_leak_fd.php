<?hh // strict

use namespace HH\Lib\OS;
use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  // Make sure that cleanup doesn't crash HHVM
  $_leak_this_fd = _OS\open('/dev/null', OS\O_RDONLY, 0);
}

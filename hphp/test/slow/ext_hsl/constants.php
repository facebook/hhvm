<?hh

use namespace HH\Lib\_Private\_OS;

<<__EntryPoint>>
function main(): void {
  // These are always in host byte order, so output should be identical on all
  // platforms
  printf("INADDR_ANY: %d 0x%08x\n", _OS\INADDR_ANY, _OS\INADDR_ANY);
  printf("INADDR_BROADCAST: %d 0x%08x\n", _OS\INADDR_BROADCAST, _OS\INADDR_BROADCAST);
  printf("INADDR_LOOPBACK: %d 0x%08x\n", _OS\INADDR_LOOPBACK, _OS\INADDR_LOOPBACK);
  printf("INADDR_NONE: %d 0x%08x\n", _OS\INADDR_NONE, _OS\INADDR_NONE);
}

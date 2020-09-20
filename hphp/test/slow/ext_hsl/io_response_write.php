<?hh

use namespace HH\Lib\_Private\_IO;

<<__EntryPoint>>
function main(): void {
  _IO\response_write("hello, ");
  print("world.\n");
  $resource = fopen('php://output', 'r');
  fwrite($resource, "Herpderp\n");
  _IO\response_write("more stuff here.\n");
  _IO\response_flush();
}

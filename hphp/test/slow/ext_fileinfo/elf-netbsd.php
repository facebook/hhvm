<?hh
// Crafted ELF with a short NetBSD-CORE PROCINFO note (descsz < 0x7c + 32).
// Plain finfo() (not MIME mode) so file_tryelf runs.

<<__EntryPoint>>
function main(): mixed {
  $finfo = new finfo();
  echo $finfo->file(__DIR__ . '/elf-netbsd.input') . "\n";
}

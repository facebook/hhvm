<?hh

<<__EntryPoint>>
function main(): void {
  $pattern = '/idonotexist/foo.XXXXXX';
  HH\Lib\OS\mkdtemp($pattern);
}

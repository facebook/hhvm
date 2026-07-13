<?hh
<<__EntryPoint>>
function main(): void {
  $fp = opendir("/");
  var_dump($fp);
  var_dump(closedir($fp));
  var_dump(readdir($fp));
  var_dump(rewinddir($fp));
  var_dump(closedir($fp));
}

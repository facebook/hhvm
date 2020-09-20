<?hh
<<__EntryPoint>>
function main(): void {
  var_dump($fp = opendir("/"));
  var_dump(closedir($fp));
  var_dump(readdir($fp));
  var_dump(rewinddir($fp));
  var_dump(closedir($fp));
}

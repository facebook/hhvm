<?hh
<<__EntryPoint>>
function main_entry(): void {
  $s = "洪仁玕";
  var_dump($s === mb_convert_encoding(mb_convert_encoding($s, "cp936", "utf8"), "utf8", "cp936"));
}

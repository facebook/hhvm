<?hh
<<__EntryPoint>>
function main_entry(): void {
  $s = "\xe6\xb4\xaa\xe4\xbb\x81\xe7\x8e\x95";
  var_dump($s === mb_convert_encoding(mb_convert_encoding($s, "cp936", "utf8"), "utf8", "cp936"));
}

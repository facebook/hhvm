<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(mb_ereg_replace("", "\xf1", "", ""));
  throw new Error("done");
}

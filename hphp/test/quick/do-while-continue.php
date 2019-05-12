<?hh

<<__EntryPoint>> function main(): void {
  do {
    var_dump("top");
    continue;
    var_dump("bottom");
  } while (false);
}

<?hh
function skip_if_no_iconv(): void {
  if (!extension_loaded("iconv")) {
    die("skip iconv extension not available\n");
  }
}

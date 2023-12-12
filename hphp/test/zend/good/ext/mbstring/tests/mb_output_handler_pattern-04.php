<?hh
<<__EntryPoint>>
function main_entry(): void {
  mb_http_output("EUC-JP");
  header("Content-Type: application/octet-stream");
  ob_start();
  ob_start('mb_output_handler');
  echo "\xe3\x83\x86\xe3\x82\xb9\xe3\x83\x88";
  ob_end_flush();
  var_dump(bin2hex(ob_get_clean()));
}

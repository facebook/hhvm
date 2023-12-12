<?hh
<<__EntryPoint>>
function main_entry(): void {
  mb_http_output("EUC-JP");
  ini_set('mbstring.http_output_conv_mimetypes', 'application');
  header("Content-Type: text/html");
  ob_start();
  ob_start('mb_output_handler');
  echo "\xe3\x83\x86\xe3\x82\xb9\xe3\x83\x88";
  ob_end_flush();
  var_dump(bin2hex(ob_get_clean()));
}

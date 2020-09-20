<?hh
<<__EntryPoint>>
function main_entry(): void {
  // TODO: Add more encoding names


  ini_set('include_path', dirname(__FILE__));
  include_once('common.inc');
  set_custom_error_handler();


  $str = mb_preferred_mime_name('sjis-win');
  echo "$str\n";

  $str = mb_preferred_mime_name('SJIS');
  echo "$str\n";

  $str = mb_preferred_mime_name('EUC-JP');
  echo "$str\n";

  $str = mb_preferred_mime_name('UTF-8');
  echo "$str\n";

  $str = mb_preferred_mime_name('ISO-2022-JP');
  echo "$str\n";

  $str = mb_preferred_mime_name('JIS');
  echo "$str\n";

  $str = mb_preferred_mime_name('ISO-8859-1');
  echo "$str\n";

  $str = mb_preferred_mime_name('UCS2');
  echo "$str\n";

  $str = mb_preferred_mime_name('UCS4');
  echo "$str\n";

  echo "== INVALID PARAMETER ==\n";
  // Invalid name
  $r = mb_preferred_mime_name('BAD_NAME');
  ($r === FALSE) ? print("OK_BAD_NAME\n") : print("NG_BAD_NAME\n");
}

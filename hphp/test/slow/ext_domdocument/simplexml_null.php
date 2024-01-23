<?hh

<<__EntryPoint>>
function main_simplexml_null(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $date = date_create_immutable();
  try {
    dom_import_simplexml($date);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
  try {
    simplexml_import_dom($date);
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

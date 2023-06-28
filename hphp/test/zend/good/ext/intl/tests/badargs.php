<?hh
function ignore_err() :mixed{}
<<__EntryPoint>>
function main_entry(): void {
  $funcs = get_extension_funcs("intl");
  set_error_handler(ignore_err<>);
  $arg = new stdClass();
  foreach($funcs as $func) {
          $rfunc = new ReflectionFunction($func);
          if($rfunc->getNumberOfRequiredParameters() == 0) {
                  continue;
          }

  		try {
  			$res = $func($arg);
  		} catch (Exception $e) { continue; }
          if($res != false) {
                  echo "$func: ";
                  var_dump($res);
          }
  }
  echo "OK!\n";
}

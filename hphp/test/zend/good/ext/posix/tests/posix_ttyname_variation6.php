<?hh



function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
        if (error_reporting() != 0) {
                // report non-silenced errors
                echo "Error: $err_no - $err_msg, $filename($linenum)\n";
        }
}



class classWithToString
{
        public function __toString() {
                return "Class A object";
        }
}

class classWithoutToString
{
}
<<__EntryPoint>>
function main_entry(): void {


  echo "*** Test substituting argument 1 with object values ***\n";
  set_error_handler(fun('test_error_handler'));

  $variation_array = darray[
    'instance of classWithToString' => new classWithToString(),
    'instance of classWithoutToString' => new classWithoutToString(),
    ];


  foreach ( $variation_array as $var ) {
    var_dump(posix_ttyname( $var  ) );
  }
}

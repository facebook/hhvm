<?hh





class classWithToString
{
        public function __toString() :mixed{
                return "Class A object";
        }
}

class classWithoutToString
{
}

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
        if (error_reporting() != 0) {
                // report non-silenced errors
                echo "Error: $err_no - $err_msg, $filename($linenum)\n";
        }
}
<<__EntryPoint>> function main(): void {
set_error_handler(test_error_handler<>);

$variation_array = dict[
  'instance of classWithToString' => new classWithToString(),
  'instance of classWithoutToString' => new classWithoutToString(),
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(fstat( $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "===DONE===\n";
}

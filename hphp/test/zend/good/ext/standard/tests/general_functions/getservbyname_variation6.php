<?hh


class classWithToString
{
        public function __toString() {
                return "Class A object";
        }
}

class classWithoutToString
{
}
<<__EntryPoint>> function main(): void {
echo "*** Test substituting argument 1 with object values ***\n";

$protocol = "tcp";


$variation_array = darray[
  'instance of classWithToString' => new classWithToString(),
  'instance of classWithoutToString' => new classWithoutToString(),
  ];


foreach ( $variation_array as $var ) {
  try { var_dump(getservbyname( $var ,  $protocol ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}

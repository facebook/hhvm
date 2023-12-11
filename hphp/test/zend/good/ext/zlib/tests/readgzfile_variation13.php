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
<<__EntryPoint>>
function main_entry(): void {


  $filename = $filename = dirname(__FILE__)."/004.txt.gz";

  $variation = dict[
    'instance of classWithToString' => new classWithToString(),
    'instance of classWithoutToString' => new classWithoutToString(),
    ];


  foreach ( $variation as $var ) {
    try { var_dump(readgzfile( $filename, $var  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }
  echo "===DONE===\n";
}

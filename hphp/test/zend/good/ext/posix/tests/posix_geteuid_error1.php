<?hh <<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$extra_args = dict[ 0 => 12312, 2 => '1234', 'string' => 'string' ];

try { var_dump( posix_geteuid( $extra_args )); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
foreach ( $extra_args as $arg )
{
    try { var_dump(posix_geteuid( $arg )); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}

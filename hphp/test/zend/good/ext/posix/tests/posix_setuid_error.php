<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$uid = '123';


$extra_arg = '12312';

try { var_dump(posix_setuid( $uid, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump(posix_setuid(  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

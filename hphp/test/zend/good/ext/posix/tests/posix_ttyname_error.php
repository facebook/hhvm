<?hh

<<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with incorrect numbers of arguments ***\n";

$fd = 'foo';
$extra_arg = 'bar'; 

try { var_dump(posix_ttyname( $fd, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

try { var_dump(posix_ttyname(  ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

<?hh
<<__EntryPoint>> function main(): void {
$string = "Life it seems, will fade away
Drifting further everyday
Getting lost within myself
Nothing matters no one else";

try { var_dump(bzcompress()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(bzcompress("1",1,1));
var_dump(bzcompress($string, 100));
var_dump(bzcompress($string, 100, -1));
var_dump(bzcompress($string, 100, 1000));
var_dump(bzcompress($string, -1, 1));

$data = bzcompress($string);
$data2 = bzcompress($string, 1, 10);

$data3 = $data2;
$data3{3} = 0;

try { var_dump(bzdecompress()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(bzdecompress(1,1,1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(bzdecompress("1",1));
var_dump(bzdecompress($data3));
var_dump(bzdecompress($data3,1));

var_dump(bzdecompress($data, -1));
var_dump(bzdecompress($data, 0));
var_dump(bzdecompress($data, 1000));
var_dump(bzdecompress($data));
var_dump(bzdecompress($data2));

echo "Done\n";
}

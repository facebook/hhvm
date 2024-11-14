<?hh


<<__EntryPoint>>
function main_47() :mixed{
$i = 0;
$a = dict[];
 list($a[$i], list($a[$i+1], $a[$i+2]), $a[$i+3]) =     vec['x', vec['y1', 'y2'], 'z'];
 var_dump($a);
}

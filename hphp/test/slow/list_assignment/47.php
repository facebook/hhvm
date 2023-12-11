<?hh


<<__EntryPoint>>
function main_47() :mixed{
$i = 0;
$a = dict[];
 list($a[$i++], list($a[$i++], $a[$i++]), $a[$i++]) =     vec['x', vec['y1', 'y2'], 'z'];
 var_dump($a);
}

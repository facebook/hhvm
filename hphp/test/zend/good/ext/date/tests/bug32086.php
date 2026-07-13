<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('America/Sao_Paulo');
$g = strtotime("2004-11-01"); echo $g, "\n";
$i = strtotime("2004-11-01 +1 day"); echo $i, "\n";
$j = strtotime("+1 day", $g); echo $j, "\n";
$k = strtotime("2004-11-02"); echo $k, "\n";
$l = strtotime("2004-11-03"); echo $l, "\n";
echo date("Y-m-d H:i:s T\n", $g);
echo date("Y-m-d H:i:s T\n", $i);
echo date("Y-m-d H:i:s T\n", $j);
echo date("Y-m-d H:i:s T\n", $k);
echo date("Y-m-d H:i:s T\n", $l);

$g = strtotime("2005-02-19"); echo $g, "\n";
$i = strtotime("2005-02-19 +1 day"); echo $i, "\n";
$j = strtotime("+1 day", $g); echo $j, "\n";
$k = strtotime("2005-02-20"); echo $k, "\n";
$l = strtotime("2005-02-21"); echo $l, "\n";
echo date("Y-m-d H:i:s T\n", $g);
echo date("Y-m-d H:i:s T\n", $i);
echo date("Y-m-d H:i:s T\n", $j);
echo date("Y-m-d H:i:s T\n", $k);
echo date("Y-m-d H:i:s T\n", $l);
}

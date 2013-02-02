<?php
# http://shootout.alioth.debian.org/u32/program.php?test=spectralnorm&lang=php&id=2
/* The Computer Language Benchmarks Game
http://shootout.alioth.debian.org/

contributed by Isaac Gouy
modified by anon
*/


function A(&$i, &$j){
   return 1.0 / (((($i+$j) * ($i+$j+1)) >> 1) + $i + 1);
}

function Av(&$n,&$v){
   global $_tpl;
   $Av = $_tpl;
   for ($i = 0; $i < $n; ++$i) {
      $sum = 0.0;
      foreach($v as $j=>$v_j) {
         $sum += A($i,$j) * $v_j;
      }
      $Av[$i] = $sum;
   }
   return $Av;
}

function Atv(&$n,&$v){
   global $_tpl;
   $Atv = $_tpl;
   for($i = 0; $i < $n; ++$i) {
      $sum = 0.0;
      foreach($v as $j=>$v_j) {
         $sum += A($j,$i) * $v_j;
      }
      $Atv[$i] = $sum;
   }
   return $Atv;
}

function AtAv(&$n,&$v){
   $tmp = Av($n,$v);
   return Atv($n, $tmp);
}

#$n = intval(($argc == 2) ? $argv[1] : 400);
$n = 400;
$u = array_fill(0, $n, 1.0);
$_tpl = array_fill(0, $n, 0.0);

for ($i=0; $i<10; $i++){
   $v = AtAv($n,$u);
   $u = AtAv($n,$v);
}

$vBv = 0.0;
$vv = 0.0;
$i = 0;
foreach($v as $val) {
   $vBv += $u[$i]*$val;
   $vv += $val*$val;
   ++$i;
}
printf("%0.9f\n", sqrt($vBv/$vv));

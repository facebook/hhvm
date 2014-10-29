<?php

/**
 * mixedbag is intended to run quickly and test a bunch of functionality.
 */

function ack($m, $n){
  if($m == 0) return $n+1;
  if($n == 0) return ack($m-1, 1);
  return ack($m - 1, ack($m, ($n - 1)));
}

function ackermann($n = 7) {
  $r = ack(3,$n);
  print "ack(3,$n): $r\n";
}

function ary($n = 50000) {
  for ($i=0; $i<$n; $i++) {
    $X[$i] = $i;
  }
  for ($i=$n-1; $i>=0; $i--) {
    $Y[$i] = $X[$i];
  }
  $last = $n-1;
  print "$Y[$last]\n";
}

function ary2($n = 50000) {
  for ($i=0; $i<$n;) {
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;

    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
  }
  for ($i=$n-1; $i>=0;) {
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;

    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
  }
  $last = $n-1;
  print "$Y[$last]\n";
}

function ary3($n = 2000) {
  for ($i=0; $i<$n; $i++) {
    $X[$i] = $i + 1;
    $Y[$i] = 0;
  }
  for ($k=0; $k<1000; $k++) {
    for ($i=$n-1; $i>=0; $i--) {
      //$Y[$i] += $X[$i];
      $Y[$i] = $Y[$i] + $X[$i];
    }
  }
  $last = $n-1;
  print "$Y[0] $Y[$last]\n";
}

function fibo_r($n){
    return(($n < 2) ? 1 : fibo_r($n - 2) + fibo_r($n - 1));
}

function fibo($n = 30) {
  $r = fibo_r($n);
  print "$r\n";
}

function hash1($n = 50000) {
  for ($i = 1; $i <= $n; $i++) {
    $X[dechex($i)] = $i;
  }
  $c = 0;
  for ($i = $n; $i > 0; $i--) {
    if ($X[dechex($i)]) { $c++; }
  }
  print "$c\n";
}

function hash2($n = 500) {
  for ($i = 0; $i < $n; $i++) {
    $hash1["foo_$i"] = $i;
    $hash2["foo_$i"] = 0;
  }
  for ($i = $n; $i > 0; $i--) {
    foreach($hash1 as $key => $value) {
      $hash2[$key] += $value;
    }
  }
  $first = "foo_0";
  $last  = "foo_".($n-1);
  print "$hash1[$first] $hash1[$last] $hash2[$first] $hash2[$last]\n";
}

function heapsort_r($n, &$ra) {
  $l = ($n >> 1) + 1;
  $ir = $n;

  while (1) {
    if ($l > 1) {
      $rra = $ra[--$l];
    } else {
      $rra = $ra[$ir];
      $ra[$ir] = $ra[1];
      if (--$ir == 1) {
        $ra[1] = $rra;
        return null;
      }
    }
    $i = $l;
    $j = $l << 1;
    while ($j <= $ir) {
      if (($j < $ir) && ($ra[$j] < $ra[$j+1])) {
        $j++;
      }
      if ($rra < $ra[$j]) {
        $ra[$i] = $ra[$j];
        $j += ($i = $j);
      } else {
        $j = $ir + 1;
      }
    }
    $ra[$i] = $rra;
  }
}

function heapsort($N = 20000) {
  global $LAST;

  for ($i=1; $i<=$N; $i++) {
    $ary[$i] = ($N - $i);
  }
  heapsort_r($N, $ary);
  var_dump($ary[$N]);
}

function mkmatrix ($rows, $cols) {
  $count = 1;
  $mx = array();
  for ($i=0; $i<$rows; $i++) {
    for ($j=0; $j<$cols; $j++) {
	    $mx[$i][$j] = $count++;
    }
  }
  return($mx);
}

function mmult ($rows, $cols, $m1, $m2) {
  $m3 = array();
  for ($i=0; $i<$rows; $i++) {
	  for ($j=0; $j<$cols; $j++) {
	    $x = 0;
	    for ($k=0; $k<$cols; $k++) {
		    $x += $m1[$i][$k] * $m2[$k][$j];
	    }
	    $m3[$i][$j] = $x;
	  }
  }
  return($m3);
}

function matrix($n = 20) {
  $SIZE = 5;
  $m1 = mkmatrix($SIZE, $SIZE);
  $m2 = mkmatrix($SIZE, $SIZE);
  while ($n--) {
    $mm = mmult($SIZE, $SIZE, $m1, $m2);
  }
  print "{$mm[0][0]} {$mm[2][3]} {$mm[3][2]} {$mm[4][4]}\n";
}

function nestedloop($n = 12) {
  $x = 0;
  for ($a=0; $a<$n; $a++)
    for ($b=0; $b<$n; $b++)
      for ($c=0; $c<$n; $c++)
        for ($d=0; $d<$n; $d++)
          for ($e=0; $e<$n; $e++)
            for ($f=0; $f<$n; $f++)
             $x++;
  print "$x\n";
}

function sieve($n = 30) {
  $count = 0;
  while ($n-- > 0) {
    $count = 0;
    $flags = range (0,8192);
    for ($i=2; $i<8193; $i++) {
      if ($flags[$i] > 0) {
        for ($k=$i+$i; $k <= 8192; $k+=$i) {
          $flags[$k] = 0;
        }
        $count++;
      }
    }
  }
  print "Count: $count\n";
}

function strcat($n = 200000) {
  $str = "";
  while ($n-- > 0) {
    $str .= "hello\n";
  }
  $len = strlen($str);
  print "$len\n";
}

function bottomUpTree($item, $depth)
{
   if($depth)
   {
      --$depth;
      $newItem = $item<<1;
      return array(
         bottomUpTree($newItem - 1, $depth),
         bottomUpTree($newItem, $depth),
         $item
      );
   }
   return array(NULL, NULL, $item);
}

function itemCheck($treeNode)
{
   $check = 0;
   do
   {
      $check += $treeNode[2];
      if(NULL == $treeNode[0])
      {
         return $check;
      }
      $check -= itemCheck($treeNode[1]);
      $treeNode = $treeNode[0];
   }
   while(TRUE);
}

function binary_trees($n = 12) {
  $minDepth = 4;
  $maxDepth = max($minDepth + 2, $n);
  $stretchDepth = $maxDepth + 1;

  $stretchTree = bottomUpTree(0, $stretchDepth);
  printf("stretch tree of depth %d\t check: %d\n",
  $stretchDepth, itemCheck($stretchTree));
  unset($stretchTree);

  $longLivedTree = bottomUpTree(0, $maxDepth);

  $iterations = 1 << ($maxDepth);
  do
  {
     $check = 0;
     for($i = 1; $i <= $iterations; ++$i)
     {
        $t = bottomUpTree($i, $minDepth);
        $check += itemCheck($t);
        unset($t);
        $t = bottomUpTree(-$i, $minDepth);
        $check += itemCheck($t);
        unset($t);
     }

     printf("%d\t trees of depth %d\t check: %d\n",
            $iterations<<1, $minDepth, $check);

     $minDepth += 2;
     $iterations >>= 2;
  }
  while($minDepth <= $maxDepth);

  printf("long lived tree of depth %d\t check: %d\n",
  $maxDepth, itemCheck($longLivedTree));
}

function Fannkuch_run($n){
   $check = 0;
   $perm = array();
   $perm1 = array();
   $count = array();
   $maxPerm = array();
   $maxFlipsCount = 0;
   $m = $n - 1;

   for ($i=0; $i<$n; $i++) $perm1[$i] = $i;
   $r = $n;

   while (TRUE) {
      // write-out the first 30 permutations
      if ($check < 30){
        for($i=0; $i<$n; $i++) echo $perm1[$i]+1;
        echo "\n";
        $check++;
      }

      while ($r != 1){ $count[$r-1] = $r; $r--; }
      if (! ($perm1[0]==0 || $perm1[$m] == $m)){
         for($i=0; $i<$n; $i++) $perm[$i] = $perm1[$i];
         $flipsCount = 0;

         while ( !(($k=$perm[0]) == 0) ) {
            $k2 = ($k+1) >> 1;
            for($i=0; $i<$k2; $i++) {
               $temp = $perm[$i]; $perm[$i] = $perm[$k-$i]; $perm[$k-$i] = $temp;
            }
            $flipsCount++;
         }

         if ($flipsCount > $maxFlipsCount) {
            $maxFlipsCount = $flipsCount;
            for($i=0; $i<$n; $i++) $maxPerm[$i] = $perm1[$i];
         }
      }

      while (TRUE) {
         if ($r == $n) return $maxFlipsCount;
         $perm0 = $perm1[0];
         $i = 0;
         while ($i < $r) {
            $j = $i + 1;
            $perm1[$i] = $perm1[$j];
            $i = $j;
         }
         $perm1[$r] = $perm0;

         $count[$r] = $count[$r] - 1;
         if ($count[$r] > 0) break;
         $r++;
      }
   }
}

function fannkuch($n = 9) {
  printf("Pfannkuchen(%d) = %d\n", $n, Fannkuch_run($n));
}

function main_function() {
  ackermann(2);
  ary(500);
  ary2(500);
  ary3(5);
  fibo(13);
  hash1(100);
  hash2(20);
  heapsort(200);
  matrix(3);
  nestedloop(3);
  sieve(1);
  strcat(80);
  binary_trees(3);
  fannkuch(6);
}

main_function();


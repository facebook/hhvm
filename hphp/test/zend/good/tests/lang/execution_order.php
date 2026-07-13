<?hh

/* strings and concat */

class strclass {
  public $str = "bad";
  public static $statstr = "bad";
}


function foo() :mixed{
  \HH\global_set('a', 'good');
  return \HH\global_get('a');
}


class c {
  public $val = 10;
  public static $stat = 20;
}
<<__EntryPoint>>
function entrypoint_execution_order(): void {

  $a = "bad";
  $b = "good";
  echo "1)";
  $a=$b; $c = $a.$a;
  echo $c;
  echo "\r\n";

  $a = "bad";
  $b = "good";
  $a=$b; $c = $a.$a;
  echo "2)";
  echo $c;
  echo "\r\n";


  $str = new strclass();
  $__l = $str->str; $str->str="good"; $c = $__l.$str->str;
  echo "3)";
  echo $c;
  echo "\r\n";

  $str->str = "bad";

  $str->str="good"; $c = $str->str.$str->str;
  echo "4)";
  echo $c;
  echo "\r\n";

  $__l = strclass::$statstr; strclass::$statstr="good"; $c = $__l.strclass::$statstr;
  echo "5)";
  echo $c;
  echo "\r\n";

  strclass::$statstr = "bad";

  strclass::$statstr="good"; $c = strclass::$statstr.strclass::$statstr;
  echo "6)";
  echo $c;
  echo "\r\n";


  \HH\global_set('a', "bad");
  echo "7)";
  echo foo() . \HH\global_get('a');
  echo "\r\n";

  \HH\global_set('a', "bad");
  echo "8)";
  echo \HH\global_get('a') . foo();
  echo "\r\n";

  /* other operators */

  $x = 1;
  $__pi = $x; $x++; $z = $x - $__pi;
  echo "9)";
  echo $z;
  echo "\r\n";

  $x = 1;
  $__pi = $x; $x++; $z = $__pi - $x;
  echo "10)";
  echo $z;
  echo "\r\n";

  $x = 1;
  ++$x; $z = $x - $x;
  echo "11)";
  echo $z;
  echo "\r\n";

  $x = 1;
  ++$x; $z = $x - $x;
  echo "12)";
  echo $z;
  echo "\r\n";


  $x = 1;
  $y = 3;
  $x=$y; $z = $x - $x;
  echo "13)";
  echo $z;
  echo "\r\n";

  $x = 1;
  $y = 3;
  $x=$y; $z = $x - $x;
  echo "14)";
  echo $z;
  echo "\r\n";


  $a = 100;
  $b = 200;
  echo "15)";
  $a=$b; echo $a + $a;
  echo "\r\n";

  $a = 100;
  $b = 200;
  echo "16)";
  $a=$b; echo $a + $a;
  echo "\r\n";


  $a = vec[100,200];
  $i = 0;

  echo "17)";
  $__i1=$i; $i++; $__i2=$i; $i++; echo $a[$__i1] + $a[$__i2];
  echo "\r\n";

  $i = -1;
  echo "18)";
  ++$i; $__i1=$i; ++$i; $__i2=$i; echo $a[$__i1] + $a[$__i2];
  echo "\r\n";

  $i = 0;
  echo "19)";
  $__l = $a[$i]; $a[$i]=400; echo $__l + $a[$i];
  echo "\r\n";


  $a[0] = 100;
  echo "20)";
  $a[$i]=400; echo $a[$i] + $a[$i];
  echo "\r\n";

  echo "21)";
  $__l = c::$stat; c::$stat=200; echo $__l + c::$stat;
  echo "\r\n";

  echo "22)";
  c::$stat=300; echo c::$stat + c::$stat;
  echo "\r\n";

  $c = new c();

  echo "23)";
  $__l = $c->val; $c->val=200; echo $__l + $c->val;
  echo "\r\n";

  echo "24)";
  $c->val=300; echo $c->val + $c->val;
  echo "\r\n";
}

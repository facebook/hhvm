<?hh
class C { public static $p; }
function f() :mixed{ return 0; }
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_STRICT);
$a = vec[dict[], dict[]];
$a[0][1] = 'good';
$a[1][1] = 'bad';

echo "\n" . '$i=f(): ';
echo $a[$i=f()][++$i];
unset($i);

echo "\n" . '$i[0]=f(): ';
$i = dict[];
echo $a[$i[0]=f()][++$i[0]];
unset($i);

echo "\n" . '$i[0][0]=f(): ';
$i = vec[dict[]];
echo $a[$i[0][0]=f()][++$i[0][0]];
unset($i);

echo "\n" . '$i->p=f(): ';
$i = new stdClass();
echo $a[$i->p=f()][++$i->p];
unset($i);

echo "\n" . '$i->p[0]=f(): ';
$i = new stdClass();
$i->p = dict[];
echo $a[$i->p[0]=f()][++$i->p[0]];
unset($i);

echo "\n" . '$i->p[0]->p=f(): ';
$i = new stdClass();
$i->p = vec[new stdClass()];
echo $a[$i->p[0]->p=f()][++$i->p[0]->p];
unset($i);

echo "\n" . 'C::$p=f(): ';
echo $a[C::$p=f()][++C::$p];

echo "\n" . 'C::$p[0]=f(): ';
C::$p = dict[];
echo $a[C::$p[0]=f()][++C::$p[0]];

echo "\n" . 'C::$p->q=f(): ';
C::$p = new stdClass;
echo $a[C::$p->q=f()][++C::$p->q];
}

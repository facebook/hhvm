<?hh
class C { public static $p; }
function f() :mixed{ return 0; }
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_STRICT);
$a = vec[dict[], dict[]];
$a[0][1] = 'good';
$a[1][1] = 'bad';

echo "\n" . '$i=f(): ';
$i=f(); $__a=$i; ++$i; $__b=$i; echo $a[$__a][$__b];
unset($i);

echo "\n" . '$i[0]=f(): ';
$i = dict[];
$i[0]=f(); $__a=$i[0]; ++$i[0]; $__b=$i[0]; echo $a[$__a][$__b];
unset($i);

echo "\n" . '$i[0][0]=f(): ';
$i = vec[dict[]];
$i[0][0]=f(); $__a=$i[0][0]; ++$i[0][0]; $__b=$i[0][0]; echo $a[$__a][$__b];
unset($i);

echo "\n" . '$i->p=f(): ';
$i = new stdClass();
$i->p=f(); $__a=$i->p; ++$i->p; $__b=$i->p; echo $a[$__a][$__b];
unset($i);

echo "\n" . '$i->p[0]=f(): ';
$i = new stdClass();
$i->p = dict[];
$i->p[0]=f(); $__a=$i->p[0]; ++$i->p[0]; $__b=$i->p[0]; echo $a[$__a][$__b];
unset($i);

echo "\n" . '$i->p[0]->p=f(): ';
$i = new stdClass();
$i->p = vec[new stdClass()];
$i->p[0]->p=f(); $__a=$i->p[0]->p; ++$i->p[0]->p; $__b=$i->p[0]->p; echo $a[$__a][$__b];
unset($i);

echo "\n" . 'C::$p=f(): ';
C::$p=f(); $__a=C::$p; ++C::$p; $__b=C::$p; echo $a[$__a][$__b];

echo "\n" . 'C::$p[0]=f(): ';
C::$p = dict[];
C::$p[0]=f(); $__a=C::$p[0]; ++C::$p[0]; $__b=C::$p[0]; echo $a[$__a][$__b];

echo "\n" . 'C::$p->q=f(): ';
C::$p = new stdClass;
C::$p->q=f(); $__a=C::$p->q; ++C::$p->q; $__b=C::$p->q; echo $a[$__a][$__b];
}

<?php
error_reporting(E_ALL & ~E_STRICT);

function f() { return 0; }
$a = array(array(), array());
$a[0][1] = 'good';
$a[1][1] = 'bad';

echo "\n" . '$i=f(): ';
echo $a[$i=f()][++$i];
unset($i);

echo "\n" . '${\'i\'}=f(): ';
echo $a[${'i'}=f()][++${'i'}];
unset(${'i'});

echo "\n" . '$i[0]=f(): ';
$i = array();
echo $a[$i[0]=f()][++$i[0]];
unset($i);

echo "\n" . '$i[0][0]=f(): ';
$i = array(array());
echo $a[$i[0][0]=f()][++$i[0][0]];
unset($i);

echo "\n" . '$i->p=f(): ';
echo $a[$i->p=f()][++$i->p];
unset($i);

echo "\n" . '$i->p->q=f(): ';
echo $a[$i->p->q=f()][++$i->p->q];
unset($i);

echo "\n" . '$i->p[0]=f(): ';
$i->p = array();
echo $a[$i->p[0]=f()][++$i->p[0]];
unset($i);

echo "\n" . '$i->p[0]->p=f(): ';
$i->p = array();
echo $a[$i->p[0]->p=f()][++$i->p[0]->p];
unset($i);

class C {
	static $p;
}

echo "\n" . 'C::$p=f(): ';
echo $a[C::$p=f()][++C::$p];

echo "\n" . 'C::$p[0]=f(): ';
C::$p = array();
echo $a[C::$p[0]=f()][++C::$p[0]];

echo "\n" . 'C::$p->q=f(): ';
C::$p = new stdclass;
echo $a[C::$p->q=f()][++C::$p->q];

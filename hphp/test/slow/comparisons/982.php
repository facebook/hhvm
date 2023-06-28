<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_982() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(true>true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, 1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, 1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, 0)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, 0)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, -1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, -1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, '1')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, '1')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, '0')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, '0')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, '-1')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, '-1')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, null)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, null)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>darray[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >darray[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray[];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(true>varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P($a >varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(true>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, 'php')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, 'php')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt(true, '')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = true;
 try { P(HH\Lib\Legacy_FIXME\gt($a, '')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(HH\Lib\Legacy_FIXME\gt(true, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "true > ''	";
 print "\n";
}

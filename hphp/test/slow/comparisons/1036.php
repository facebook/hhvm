<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_1036() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, true)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, true)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, false)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, false)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, 1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, 1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, 0)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, 0)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, -1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, -1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, '1')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, '1')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, '0')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, '0')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, '-1')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, '-1')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=darray[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=darray[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray[];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null>=varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a >=varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(null>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, 'php')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, 'php')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(null, '')); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P(HH\Lib\Legacy_FIXME\gte($a, '')); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(HH\Lib\Legacy_FIXME\gte(null, $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null >= ''	";
 print "\n";
}

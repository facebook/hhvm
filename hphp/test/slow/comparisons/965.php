<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_965() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', true)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, true)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', false)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, false)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', 1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, 1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', 0)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, 0)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', -1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, -1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\lt('0', null)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P(HH\Lib\Legacy_FIXME\lt($a, null)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(HH\Lib\Legacy_FIXME\lt('0', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\lt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<darray[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <darray[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray[];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('0'<''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '0';
 try { P($a <''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P('0'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'0' < ''	";
 print "\n";
}

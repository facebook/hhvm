<?hh

<<__NEVER_INLINE>> function P(bool $v) { print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_958() {
$i = 0;
 print ++$i;
 print "\t";
 try { P('1.2'<true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P('1.2'<''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '1.2';
 try { P($a <''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P('1.2'<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'1.2' < ''	";
 print "\n";
}

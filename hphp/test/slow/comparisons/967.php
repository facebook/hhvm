<?hh

<<__NEVER_INLINE>> function P(bool $v) { print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_967() {
$i = 0;
 print ++$i;
 print "\t";
 try { P(null<true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(null<''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = null;
 try { P($a <''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(null<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "null < ''	";
 print "\n";
}

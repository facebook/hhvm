<?hh

<<__NEVER_INLINE>> function P(bool $v) { print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_973() {
$i = 0;
 print ++$i;
 print "\t";
 try { P(varray['a']<true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(varray['a']<''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = varray['a'];
 try { P($a <''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(varray['a']<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') < ''	";
 print "\n";
}

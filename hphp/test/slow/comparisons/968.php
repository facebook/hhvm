<?hh

<<__NEVER_INLINE>> function P(bool $v) { print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_968() {
$i = 0;
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P($a <''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])<$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array() < ''	";
 print "\n";
}

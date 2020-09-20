<?hh

<<__NEVER_INLINE>> function P(bool $v) { print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_1006() {
$i = 0;
 print ++$i;
 print "\t";
 try { P(false<=true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<='1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <='1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<='0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <='0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<='-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <='-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]);
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[1];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[2];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['1'];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=darray['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['0' => '1'];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray['a'];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=darray['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=darray['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['b' => 1];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=darray['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = darray['a' => 1, 'b' => 2];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray[darray['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['a' => 1]];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=varray[darray['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = varray[darray['b' => 1]];
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<='php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <='php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(false<=''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = false;
 try { P($a <=''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(false<=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a <=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "false <= ''	";
 print "\n";
}

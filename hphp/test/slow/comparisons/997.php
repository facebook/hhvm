<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_997() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>dict[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >dict[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict[];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[1];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[2];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['1'];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>dict['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >dict['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['0' => '1'];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['a'];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['b' => 1];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1, 'b' => 2];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['a' => 1]];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['b' => 1]];
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(dict['a' => 1]>''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = dict['a' => 1];
 try { P($a >''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(dict['a' => 1]>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a' => 1) > ''	";
 print "\n";
}

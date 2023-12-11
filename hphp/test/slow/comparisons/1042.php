<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_1042() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(vec['a']>=true); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=true); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=false); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=false); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=0); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=0); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=-1); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=-1); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>='1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >='1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>='0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >='0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>='-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >='-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=null); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=null); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=dict[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=dict[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict[];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(vec['a'], vec[1])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P(HH\Lib\Legacy_FIXME\gte($a, vec[1])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[1];
 try { P(HH\Lib\Legacy_FIXME\gte(vec['a'], $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gte(vec['a'], vec[2])); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P(HH\Lib\Legacy_FIXME\gte($a, vec[2])); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[2];
 try { P(HH\Lib\Legacy_FIXME\gte(vec['a'], $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gte($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=vec['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=vec['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['1'];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=dict[0 => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=dict[0 => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict[0 => '1'];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array(0 => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=vec['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=vec['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['a'];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['b' => 1];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1, 'b' => 2];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['a' => 1]];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['b' => 1]];
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>='php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >='php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(vec['a']>=''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = vec['a'];
 try { P($a >=''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(vec['a']>=$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >=$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "array('a') >= ''	";
 print "\n";
}

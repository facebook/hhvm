<?hh

<<__NEVER_INLINE>> function P(bool $v) :mixed{ print $v ? 'Y' : 'N'; }

<<__EntryPoint>>
function main_1003() :mixed{
$i = 0;
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', true)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, true)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = true;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > true	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', false)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, false)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = false;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > false	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', 1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, 1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 1;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > 1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', 0)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, 0)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 0;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > 0	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', -1)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, -1)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = -1;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > -1	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>'1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >'1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '1';
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > '1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>'0'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >'0'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '0';
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > '0'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>'-1'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >'-1'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '-1';
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > '-1'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(HH\Lib\Legacy_FIXME\gt('', null)); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P(HH\Lib\Legacy_FIXME\gt($a, null)); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = null;
 try { P(HH\Lib\Legacy_FIXME\gt('', $b)); } catch (Throwable $_) { print 'E'; }
 try { P(HH\Lib\Legacy_FIXME\gt($a, $b)); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > null	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>dict[]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >dict[]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict[];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array()	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec[1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec[1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[1];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array(1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec[2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec[2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[2];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array(2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec['1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec['1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['1'];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>dict['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >dict['0' => '1']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['0' => '1'];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('0' => '1')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec['a']); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec['a']); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec['a'];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('a')	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >dict['a' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('a' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >dict['b' => 1]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['b' => 1];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('b' => 1)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >dict['a' => 1, 'b' => 2]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = dict['a' => 1, 'b' => 2];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array('a' => 1, 'b' => 2)	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec[dict['a' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['a' => 1]];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array(array('a' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >vec[dict['b' => 1]]); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = vec[dict['b' => 1]];
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > array(array('b' => 1))	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>'php'); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >'php'); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = 'php';
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > 'php'	";
 print "\n";
 print ++$i;
 print "\t";
 try { P(''>''); } catch (Throwable $_) { print 'E'; }
 $a = 1;
 $a = 't';
 $a = '';
 try { P($a >''); } catch (Throwable $_) { print 'E'; }
 $b = 1;
 $b = 't';
 $b = '';
 try { P(''>$b); } catch (Throwable $_) { print 'E'; }
 try { P($a >$b); } catch (Throwable $_) { print 'E'; }
 print "\t";
 print "'' > ''	";
 print "\n";
}

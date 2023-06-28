<?hh

function quote_default($f = '"\'"') :mixed{}
function zero_default($f = "x\0x") :mixed{}
function zero12_default($f = "x\00012x") :mixed{}
function misc_default($f = "blah\n'\"\$") :mixed{}
function float_default($f = 1.234) :mixed{}
function float_default2($f = 1.0) :mixed{}
function float_defaulte($f = 2e100) :mixed{}
class Foo { const BLAH = 'blah'; }
function clscns_default($f = Foo::BLAH) :mixed{}
function id($x) :mixed{ return $x; }

function do_function($fn) :mixed{
  echo "\n$fn\n";
  $param = id(new ReflectionFunction($fn))->getParameters()[0];
  $str = $param->getDefaultValueText();
  echo "$str\n";
  var_dump($param->getDefaultValue());
  $fn .= '_eval';
  eval(sprintf('function %s($v = %s) { var_dump($v); }',
               $fn, $str));
  $fn();
}

<<__EntryPoint>> function main(): void {
  do_function('quote_default');
  do_function('clscns_default');
  do_function('zero_default');
  do_function('zero12_default');
  do_function('misc_default');
  do_function('float_default');
  do_function('float_default2');
  do_function('float_defaulte');
}

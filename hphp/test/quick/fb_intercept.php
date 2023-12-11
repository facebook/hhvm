<?hh
function mkh($c) :mixed{ return ($_1, $_2, inout $_3) ==> shape('value' => $c()); }
function handler($name, $obj, inout $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape('value' => null);
}

function passthrough_handler($name, $obj, inout $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args);
  echo "---------------\n";
  return shape();
}

function frap($arg) :mixed{
  echo "frap $arg\n";
}

function test_standard_function() :mixed{
  echo '---------- ', __FUNCTION__, ' ----------', "\n";
  // Call once normally first; make sure translator can handle it
  frap('claptrap');

  // Intercept a function
  fb_intercept2('frap', 'handler');
  frap('claptrap');
  call_user_func(frap<>, 'callfunc');

  fb_intercept2('frap', 'passthrough_handler');
  frap('claptrap');
  call_user_func(frap<>, 'callfunc');

  // Replace with closure
  fb_intercept2('frap', mkh(function () { echo "Closure! wooooo\n"; }));
  frap('claptrap');

  // Reset
  fb_intercept2('frap', null);
  frap('claptrap');
}

function var_frap($arg, ...$rest) :mixed{
  echo "var_frap $arg "; var_dump($rest);
}

function test_variadic_function() :mixed{
  echo '---------- ', __FUNCTION__, ' ----------', "\n";

  // Call once normally first; make sure translator can handle it
  var_frap('claptrap', 'blah');

  // Intercept a function
  fb_intercept2('var_frap', 'handler');
  var_frap('claptrap', 'blah');
  call_user_func(var_frap<>, 'callfunc');

  fb_intercept2('var_frap', 'passthrough_handler');
  var_frap('claptrap', 'blah');
  call_user_func(var_frap<>, 'callfunc');

  // Replace with closure
  fb_intercept2('var_frap', mkh(function () { echo "Closure! wooooo\n"; }));
  var_frap('claptrap', 'blah');

  // Reset
  fb_intercept2('var_frap', null);
  var_frap('claptrap', 'blah');
}

class Blark {
  public static function sfrap() :mixed{
    echo "static frap " . static::class . "\n";
  }
  public function frap() :mixed{
    echo "non-static frap\n";
  }
}

class SubBlark extends Blark {}
class SubBlark2 extends Blark {}

function test_methods() :mixed{
  echo '---------- ', __FUNCTION__, ' ----------', "\n";

  // Intercept static method
  fb_intercept2('SubBlark2::sfrap', 'handler');
  Blark::sfrap();
  call_user_func(vec['Blark', 'sfrap']);
  SubBlark::sfrap();
  call_user_func(vec['SubBlark', 'sfrap']);
  SubBlark2::sfrap();
  call_user_func(vec['SubBlark2', 'sfrap']);

  fb_intercept2('Blark::sfrap', 'handler');
  Blark::sfrap();
  call_user_func(vec['Blark', 'sfrap']);
  SubBlark::sfrap();
  call_user_func(vec['SubBlark', 'sfrap']);
  SubBlark2::sfrap();
  call_user_func(vec['SubBlark2', 'sfrap']);

  fb_intercept2('Blark::sfrap', 'passthrough_handler');
  Blark::sfrap();
  call_user_func(vec['Blark', 'sfrap']);

  // Intercept non-static method
  $b = new Blark();
  fb_intercept2('Blark::frap', 'handler');
  $b->frap();
  call_user_func(vec[$b, 'frap']);

  fb_intercept2('Blark::frap', 'passthrough_handler');
  $b->frap();
  call_user_func(vec[$b, 'frap']);

  // MULTI-INTERCEPT!
  fb_intercept2('frap', 'handler');
  fb_intercept2('handler', 'passthrough_handler');
  frap('claptrap');

  // Reset all
  fb_intercept2('frap', null);
  fb_intercept2('Blark::sfrap', null);
  fb_intercept2('Blark::frap', null);
  frap('claptrap');
  Blark::sfrap();
  $b->frap();
}

<<__EntryPoint>> function main(): void {
  test_standard_function();
  test_variadic_function();
  test_methods();
}

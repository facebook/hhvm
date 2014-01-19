<?

$_SERVER['PHP_ROOT'] = dirname(__FILE__)."../../..";

function h() {
    var_dump(fb_debug_backtrace());
}

class C {
  static function f() {
    h();
  }
}

/**
 * fb_debug_backtrace - returns the current backtrace minus junk.
 *
 * Does some filtering to remove the all the function that
 * we generally don't care about.
 *
 * @return - function backtrace as an array
 *
 * @author epriestley
 */
function fb_debug_backtrace($skip_top_libcore=true, $bt=null) {
  static $real = null;

  if ($real === null) {
    $real = strlen(realpath($_SERVER['PHP_ROOT']).'/');
  }

  if (!$bt) {  // fb_handle_error defaults to array() in Zend PHP
    $bt = debug_backtrace();
    // Remove fb_debug_backtrace from the backtrace
    array_shift($bt);
  }

  // Remove all lib/core functions at the top of the stack
  if ($skip_top_libcore === true) {
    while (isset($bt[0]['file']) &&
           substr(realpath($bt[0]['file']), $real, 9) === 'lib/core/') {
      array_shift($bt);
    }
  }

  //  We never care about whatever junk actually called
  //  fb_debug_backtrace, or the second-level function which is always
  //  some cache wrapper.
  $last_file = 'lib/intern/dummy.php';
  $last_line = 1;
  for ($k = count($bt) - 1; $k >= 0; $k--) {
    if (isset($bt[$k]['file'])) {
      $real_file = substr(realpath($bt[$k]['file']), $real);
      $last_file = $real_file;
      $last_line = $bt[$k]['line'];
    } else {
      $real_file = $last_file;
      $bt[$k]['line'] = $last_line;
    }

    $fn = '';
    $class = '';
    if (isset($bt[$k]['function'])) {
      $fn = $bt[$k]['function'];
      if (isset($bt[$k]['class'])) {
        $type = '->';
        if (isset($bt[$k]['type'])) {
          $type = $bt[$k]['type'];
        }
        $fn = $bt[$k]['class'].$type.$fn;
      }
    }

    $bt[$k] = $real_file.'@'.$bt[$k]['line'].'@'.$fn
      .'@'.$class;
  }

  return array_values($bt);
}

function g() {
  C::f();
}
g();


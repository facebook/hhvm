<?hh // partial

/**
 * This function is intended to be used as a default implementation for
 * __autoload(). If nothing else is specified and spl_autoload_register()
 * is called without any parameters then this functions will be used for
 * any later call to __autoload().
 */
function spl_autoload(string $class, ?string $extensions = null) {
  if ($extensions === null) {
    $extensions = spl_autoload_extensions();
  }
  $extensions = explode(',', $extensions);
  // Lowercase, convert namespace separators to path separators
  $normalized = str_replace(
    '\\',
    '/',
    strtolower($class),
  );
  foreach ($extensions as $ext) {
    $filename = $normalized.$ext;
    $old = error_reporting();
    try {
      error_reporting(0);
      include($filename);
    } finally {
      if (error_reporting() === 0) {
        error_reporting($old);
      }
    }
    if (class_exists($class)) {
      return;
    }
  }
}

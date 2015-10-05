<?hh
require_once __DIR__.'/../Framework.php';

class Wordpress extends Framework {
  <<Override>>
  protected function extraPostComposer(): void {
    verbose("Setting up database configuration.\n");
    $config = <<<CONFIG
<?php

/* Path to the WordPress codebase you'd like to test. Add a backslash in the end. */
define( 'ABSPATH', dirname( __FILE__ ) . '/src/' );

// Multisite tests
define( 'WP_TESTS_MULTISITE', true );

// Test with WordPress debug mode (default).
define( 'WP_DEBUG', true );

// ** MySQL settings ** //
define( 'DB_NAME', 'wordpress' );
define( 'DB_USER', 'root' );
define( 'DB_PASSWORD', '' );
define( 'DB_HOST', 'localhost' );
define( 'DB_CHARSET', 'utf8' );
define( 'DB_COLLATE', '' );

\$table_prefix  = 'wptests_';   // Only numbers, letters, and underscores please!

define( 'WP_TESTS_DOMAIN', 'example.org' );
define( 'WP_TESTS_EMAIL', 'admin@example.org' );
define( 'WP_TESTS_TITLE', 'Test Blog' );

define( 'WP_PHP_BINARY', 'hhvm' );

define( 'WPLANG', '' );
CONFIG;
    file_put_contents($this->getInstallRoot() . '/wp-tests-config.php', $config);
  }
}

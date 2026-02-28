--TEST--
SQLite
--SKIPIF--
<?hh # vim:ft=php
if (!extension_loaded('pdo_sqlite')) print 'skip'; ?>
--REDIRECTTEST--
function redirecttest() {
	return dict[
		'ENV' => dict['PDOTEST_DSN' => 'sqlite::memory:'],
		'TESTS' => 'ext/pdo/tests'
	];
}

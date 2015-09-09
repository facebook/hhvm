# PHP7 Compatibility Tests

Tests in this suite are related to introducing PHP7 compatibility which has BC
('backwards-compatible') breakage. For PHP7 backports which are not BC
breakers, please see `slow/php7_backported`.

The layout here is as follows:

* `./5` : Tests demonstrating the PHP5 behavior. This is where you'll start
  writing new tests.

* `./7` : Test stubs which include the corresponding PHP5 test, but expect
  different output. For your convenience, you can create these stubs with
  `./make_7_tests.sh`. All tests here run with `hhvm.php7.all = 1` per
  config.ini.

* `./zend7` : Manually-imported specification tests. Consult the RFC patch in
  php-src and find relevant test updates. Download those tests and use
  `hphp/test/tools/import_zend_test.py --local <filename>` to convert them to
  local tests format. All tests here run with `hhvm.php7.all = 1` as well.

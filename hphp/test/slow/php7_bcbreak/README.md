# PHP7 Compatibility Tests

Tests in this suite are related to introducing PHP7 compatibility which has BC
('backwards-compatible') breakage. For PHP7 backports which are not BC
breakers, please see `slow/php7_backported`.

Tests in this directory are for new HHVM-specific tests which test
backwards-compatibility-breaking changes in PHP7.

* Tests which were imported from PHP7 belong in `slow/php7_backported`.

* HHVM-specific tests which do not test BC-breaking functionality can just have
  their own directories, such as `slow/fun-feature/function1.php` etc.

The layout here is as follows:

* `./5` : Tests demonstrating the PHP5 behavior. This is where you'll start
  writing new tests.

* `./7` : Test stubs which include the corresponding PHP5 test, but expect
  different output. For your convenience, you can create these stubs with
  `./make_7_tests.sh`. We deliberately do *not* set `hhvm.php7.all` here --
  please set the appropriate granular config option either in a directory INI
  (for groups of tests) or in a per-test INI (for a single test).

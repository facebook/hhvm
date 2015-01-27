Usage
=====

    $ hhvm run.php --all
    $ hhvm run.php doctrine2
    $ hhvm run.php --verbose doctrine2

For more information, run:

    $ hhvm run.php --help

Adding a new framework
======================

- Update the frameworks.yml file with the framework information
- Run `hphp/test/frameworks/run.php framework-name`. A file is created for you in `hphp/test/frameworks/results/` called `framework-name.expect`, add that file to your commit as well

Requirements
============

- HHVM
- NodeJS (used by Guzzle tests, among others)
- Git
- Mercurial

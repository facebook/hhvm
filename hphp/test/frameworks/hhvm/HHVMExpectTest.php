<?php

abstract class HHVMExpectTest extends PHPUnit_Framework_TestCase {
  protected function runTests($runner_args, $paths) {
    $runner = self::getTestRoot().'/run';
    $paths = implode(
      ' ',
      array_map('escapeshellarg', $paths)
    );
    $cmd = sprintf('TRAVIS=1 %s --no-fun %s %s', $runner, $runner_args, $paths);
    $output = `$cmd`;
    $lines = explode("\n", trim($output));
    $this->assertSame(
      'All tests passed.',
      end($lines)
    );
  }

  protected function getChunkedTestPaths($tests_dir) {
    $tests = array();
    $dir_it = new RecursiveDirectoryIterator($tests_dir);
    foreach (new RecursiveIteratorIterator($dir_it) as $path => $object) {
      if ($object->getExtension() == 'php') {
        $tests[] = $path;
      }
    }

    // 32 is an arbitrary limit to avoid going above the maximum command
    // line length
    return array_chunk($tests, 32);
  }

  protected static function getTestRoot() {
    // hphp/test/
    return realpath(__DIR__.'/../../');
  }
}

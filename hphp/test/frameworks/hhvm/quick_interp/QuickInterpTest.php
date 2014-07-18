<?php

require(__DIR__.'/../HHVMExpectTest.php');

final class QuickInterpTest extends HHVMExpectTest {
  /**
   * @dataProvider quickTestsProvider
   */
  public function testQuickInterp(...$paths) {
    $this->runTests('-m interp', $paths);
  }

  public function quickTestsProvider() {
    return $this->getChunkedTestPaths($this->getTestRoot().'/quick');
  }
}

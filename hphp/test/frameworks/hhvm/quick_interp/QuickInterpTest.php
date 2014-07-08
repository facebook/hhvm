<?php

require(__DIR__.'/../HHVMExpectTest.php');

final class QuickInterpTest extends HHVMExpectTest {
  /**
   * @dataProvider quickTestsProvider
   */
  public function testQuickInterp(/* ... $paths */) {
    $this->runTests('-m interp', func_get_args());
  }

  public function quickTestsProvider() {
    return $this->getChunkedTestPaths($this->getTestRoot().'/quick');
  }
}

<?php

require(__DIR__.'/../HHVMExpectTest.php');

final class HHVMQuickTest extends HHVMExpectTest {
  /**
   * @dataProvider quickTestsProvider
   */
  public function testQuickInterp(...$paths) {
    $this->runTests('-m interp', $paths);
  }

  /**
   * @dataProvider quickTestsProvider
   */
  public function testQuickJit(...$paths) {
    $this->runTests('-m jit', $paths);
  }

  public function quickTestsProvider() {
    return $this->getChunkedTestPaths($this->getTestRoot().'/quick');
  }
}

<?php

/**
 * Helps application inserting an artificial frame in xhprof's reporting.
 */
class XhprofFrame {
  public function __construct($name) {
    xhprof_frame_begin($name);
  }
  public function __destruct() {
    xhprof_frame_end();
  }
}

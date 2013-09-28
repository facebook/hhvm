<?php

include 'debug_backtrace_trait_helper.inc';
class C extends P {
  use T;
}
(new C)->bar(new stdClass);
(new C)->bar(12);

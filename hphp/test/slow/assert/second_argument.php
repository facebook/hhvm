<?php
assert_options(ASSERT_ACTIVE,1);
assert_options(ASSERT_WARNING,1);

assert(false, "asserting that false is true");
assert(true, "foo");
assert(1===array(), "This doesn't make any sense.\n\"");

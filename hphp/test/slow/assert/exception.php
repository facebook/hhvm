<?php

class C extends Exception {}
class D {
  public function __toString() {
    return 'D to string';
  }
}

function exn($e) {
  printf("%s: %s\n", get_class($e), $e->getMessage());
}

assert_options(ASSERT_ACTIVE, 1);
assert_options(ASSERT_EXCEPTION, 1);

try { assert(false); } catch (Exception $e) { exn($e); }
try { assert(false, 'hi'); } catch (Exception $e) { exn($e); }
try { assert(false, new C); } catch (Exception $e) { exn($e); }
try { assert(false, new D); } catch (Exception $e) { exn($e); }

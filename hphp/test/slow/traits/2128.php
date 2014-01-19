<?php

final class Foo {
  use Bar;
  private static $a = 3;
}
trait Bar {
  private static $a = 3;
}
echo "Done\n";

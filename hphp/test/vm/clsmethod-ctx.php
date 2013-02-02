<?php

class a {
  private static function priv() {
    echo "Private in a\n";
  }

  public static function pub() {
    a::priv();
  }
}

function main() {
  a::pub();
  a::priv();
}
main();

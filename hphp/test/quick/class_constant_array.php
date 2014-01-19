<?php

echo "Shouldn't see me\n";
if (false) {
  class c{
    const ArrayConst = array(1, 2, 'hi');
  }
}
echo "or me\n";

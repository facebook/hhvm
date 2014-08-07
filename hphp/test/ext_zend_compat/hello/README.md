## What
This is a simple hello zend extension, taken from
  http://devzone.zend.com/303/extension-writing-part-i-introduction-to-php-and-zend/

Build:
  bash ../../../tools/hphpize/hphpize
  cmake .
  make

Examine products:
  nm -u hello.so | c++filt

Clean up build junk:
  bash ./clean.sh

# HipHop VM for PHP

HipHop VM (HHVM) is a new open-source virtual machine designed for executing programs written in PHP. HHVM uses a just-in-time compilation approach to achieve superior performance while maintaining the flexibility that PHP developers are accustomed to. HipHop VM (and before it HPHPc) has realized > 5x increase in throughput for Facebook compared with Zend PHP 5.2.

HipHop is most commonly run as a standalone server, replacing both Apache and modphp.

## Installing

You can install a [prebuilt package](https://github.com/facebook/hiphop-php/wiki#installing-pre-built-packages-for-hhvm) or [compile from source](https://github.com/facebook/hiphop-php/wiki#building-hhvm).

## Contributing

We'd love to have your help in making HipHop better. If you run into problems, please open an [issue](http://github.com/facebook/hiphop-php/), or better yet, fork us and send a pull request.

If you want to help but don't know where to start, try fixing some of the [Zend tests that don't pass](https://github.com/facebook/hiphop-php/tree/master/hphp/test/zend/bad). You can run them with [hphp/test/run](https://github.com/facebook/hiphop-php/blob/master/hphp/test/run). When they work, move them to [zend/good](https://github.com/facebook/hiphop-php/tree/master/hphp/test/zend/good) and send a pull request.

Before changes can be accepted a [Contributors Licensing Agreement](http://developers.facebook.com/opensource/cla) must be signed and returned.

## Licence

HipHop VM is licensed under the PHP and Zend licenses except as otherwise noted.

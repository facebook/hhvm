# HipHop VM for PHP [![Build Status](https://travis-ci.org/facebook/hiphop-php.png?branch=master)](https://travis-ci.org/facebook/hiphop-php)

HipHop VM (HHVM) is a new open-source virtual machine designed for executing programs written in PHP. HHVM uses a just-in-time compilation approach to achieve superior performance while maintaining the flexibility that PHP developers are accustomed to. HipHop VM (and before it HPHPc) has realized > 5x increase in throughput for Facebook compared with Zend PHP 5.2.

HipHop is most commonly run as a standalone server, replacing both Apache and modphp.

## FAQ

Our [FAQ](https://github.com/facebook/hiphop-php/wiki/FAQ) has answers to many common questions about HHVM, from [general questions](https://github.com/facebook/hiphop-php/wiki/FAQ#general) to questions geared towards those that want to [use](https://github.com/facebook/hiphop-php/wiki/FAQ#users) or [contribute]((https://github.com/facebook/hiphop-php/wiki/FAQ#contributors) to HHVM.

## Installing

You can install a [prebuilt package](https://github.com/facebook/hiphop-php/wiki#installing-pre-built-packages-for-hhvm) or [compile from source](https://github.com/facebook/hiphop-php/wiki#building-hhvm).

## Running

You can run standalone programs just by passing them to hhvm: `hhvm my_script.php`.

HipHop bundles in a webserver. So if you want to run on port 80 in the current directory:

```
sudo hhvm -m server
```

For anything more complicated, you'll want to make a [config.hdf](https://github.com/facebook/hiphop-php/wiki/Runtime-options#server) and run `sudo hhvm -m server -c config.hdf`.

## Contributing

We'd love to have your help in making HipHop better. 

Before changes can be accepted a [Contributor License Agreement](http://developers.facebook.com/opensource/cla) ([pdf](https://github.com/facebook/hiphop-php/raw/master/hphp/doc/FB_Individual_CLA.pdf) - print, sign, scan, link) must be signed.

If you run into problems, please open an [issue](http://github.com/facebook/hiphop-php/issues), or better yet, fork us and send a pull request. Join us on [#hhvm on freenode](http://webchat.freenode.net/?channels=hhvm).

If you want to help but don't know where to start, try fixing some of the [Zend tests that don't pass](hphp/test/zend/bad). You can run them with [hphp/test/run](hphp/test/run). When they work, move them to [zend/good](hphp/test/zend/good) and send a pull request.

All the open issues tagged [Zend incompatibility](https://github.com/facebook/hiphop-php/issues?labels=zend+incompatibility&page=1&state=open) are real issues reported by the community in existing PHP code and [frameworks](https://github.com/facebook/hiphop-php/wiki/OSS-PHP-Frameworks-Unit-Testing:-General) that could use some attention. Please add appropriate test cases as you make changes; see [here](hphp/test) for more information. Travis-CI is integrated with this GitHub project and will provide test results automatically on all pulls.


## License

HipHop VM is licensed under the PHP and Zend licenses except as otherwise noted.

## Reporting Crashes

See [Reporting Crashes](https://github.com/facebook/hiphop-php/wiki/Reporting-Crashes) for helpful tips on how to report crashes in an actionable manner.

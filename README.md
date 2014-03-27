# HHVM [![Build Status](https://travis-ci.org/facebook/hhvm.png?branch=master)](https://travis-ci.org/facebook/hhvm)

HHVM (aka the HipHop Virtual Machine) is an open-source virtual machine designed for executing programs written in [Hack](http://hacklang.org) and PHP. HHVM uses a just-in-time compilation approach to achieve superior performance while maintaining the flexibility that PHP developers are accustomed to. To date, HHVM (and its predecessor HPHPc before it) has realized over a 9x increase in web request throughput and over a 5x reduction in memory consumption for Facebook compared with the PHP 5.2 engine + APC.

HHVM should be used together with a FastCGI-based webserver like [nginx](https://github.com/facebook/hhvm/wiki/FastCGI#making-it-work-with-nginx) or [apache](https://github.com/facebook/hhvm/wiki/FastCGI#making-it-work-with-apache).

* HHVM page: http://hhvm.com
* Hack page: http://hacklang.org

## FAQ

Our [FAQ](https://github.com/facebook/hhvm/wiki/FAQ) has answers to many common questions about HHVM, from [general questions](https://github.com/facebook/hhvm/wiki/FAQ#general) to questions geared towards those that want to [use](https://github.com/facebook/hhvm/wiki/FAQ#users) or [contribute](https://github.com/facebook/hhvm/wiki/FAQ#contributors) to HHVM.


## Installing

You can install a [prebuilt package](https://github.com/facebook/hhvm/wiki#installing-pre-built-packages-for-hhvm) or [compile from source](https://github.com/facebook/hhvm/wiki#building-hhvm).


## Running

You can run standalone programs just by passing them to hhvm: `hhvm my_script.php`.

If you want to host a website: 
* Install your favorite webserver
* Install our [package](https://github.com/facebook/hhvm/wiki#installing-pre-built-packages-for-hhvm)
* Start your webserver
* Run `sudo /etc/init.d/hhvm start`
* Visit your site at http://.../index.php

## Contributing

We'd love to have your help in making HHVM better.

Before changes can be accepted a [Contributor License Agreement](http://developers.facebook.com/opensource/cla) ([pdf](https://github.com/facebook/hhvm/raw/master/hphp/doc/FB_Individual_CLA.pdf) - print, sign, scan, link) must be signed.

If you run into problems, please open an [issue](http://github.com/facebook/hhvm/issues), or better yet, [fork us and send a pull request](https://github.com/facebook/hhvm/pulls). Join us on [#hhvm on freenode](http://webchat.freenode.net/?channels=hhvm).

If you want to help but don't know where to start, try fixing some of the [PHP5 tests that don't pass](hphp/test/zend/bad). You can run them with [hphp/test/run](hphp/test/run). When they work, move them to [zend/good](hphp/test/zend/good) and send a pull request.

All the open issues tagged [php5 incompatibility](https://github.com/facebook/hhvm/issues?labels=php5+incompatibility&page=1&state=open) are real issues reported by the community in existing PHP code and [frameworks](https://github.com/facebook/hhvm/wiki/OSS-PHP-Frameworks-Unit-Testing:-General) that could use some attention. Please add appropriate test cases as you make changes; see [here](hphp/test) for more information. Travis-CI is integrated with this GitHub project and will provide test results automatically on all pulls.


## License

HHVM is licensed under the PHP and Zend licenses except as otherwise noted.

## Reporting Crashes

See [Reporting Crashes](https://github.com/facebook/hhvm/wiki/Reporting-Crashes) for helpful tips on how to report crashes in an actionable manner.

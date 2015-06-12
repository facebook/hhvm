# HHVM

[HHVM page](http://hhvm.com) |
[Hacklang page](http://hacklang.org) |
[General group](https://www.facebook.com/groups/hhvm.general/) |
[Dev group](https://www.facebook.com/groups/hhvm.dev/) |
[Twitter](http://twitter.com/HipHopVM)

HHVM (aka the HipHop Virtual Machine) is an open-source virtual machine designed for executing programs written in [Hack](http://hacklang.org) and PHP. HHVM uses a just-in-time compilation approach to achieve superior performance while maintaining the flexibility that PHP developers are accustomed to. To date, HHVM (and its predecessor HPHPc before it) has realized over a 9x increase in web request throughput and over a 5x reduction in memory consumption for Facebook compared with the PHP 5.2 engine + APC.

HHVM should be used together with a FastCGI-based webserver like [nginx](https://github.com/facebook/hhvm/wiki/FastCGI#making-it-work-with-nginx) or [apache](https://github.com/facebook/hhvm/wiki/FastCGI#making-it-work-with-apache).

## Reporting and Fixing Security Issues

Please do not open GitHub issues or pull requests - this makes the problem
immediately visible to everyone, including malicious actors. Security issues in
HHVM can be safely reported via HHVM's Whitehat Bug Bounty program:

https://www.facebook.com/whitehat

Facebook's security team will triage your report and determine whether or not
is it eligible for a bounty under our program.

## FAQ

Our [FAQ](https://github.com/facebook/hhvm/wiki/FAQ) has answers to many common questions about HHVM, from [general questions](https://github.com/facebook/hhvm/wiki/FAQ#general) to questions geared towards those that want to [use](https://github.com/facebook/hhvm/wiki/FAQ#users) or [contribute](https://github.com/facebook/hhvm/wiki/FAQ#contributors) to HHVM.


## Installing

If you're new, try our [getting started overview](https://github.com/facebook/hhvm/wiki/Getting-Started).

You can install a [prebuilt package](https://github.com/facebook/hhvm/wiki/Prebuilt%20Packages%20for%20HHVM) or [compile from source](https://github.com/facebook/hhvm/wiki/Building%20and%20Installing%20HHVM).


## Running

You can run standalone programs just by passing them to hhvm: `hhvm my_script.php`.

If you want to host a website:
* Install your favorite webserver
* Install our [package](https://github.com/facebook/hhvm/wiki/Prebuilt%20Packages%20for%20HHVM)
* Start your webserver
* Run `sudo /etc/init.d/hhvm start`
* Visit your site at http://.../index.php

Our [getting started overview](https://github.com/facebook/hhvm/wiki/Getting-Started) provides a slightly more detailed introduction as well as links to more information.


## Contributing

We'd love to have your help in making HHVM better. If you're interested, please read our [guide to contributing](CONTRIBUTING.md).

## License

HHVM is licensed under the PHP and Zend licenses except as otherwise noted.

The Hack typechecker (`hphp/hack`) is licensed under the BSD license (`hphp/hack/LICENSE`) with an additional grant of patent rights (`hphp/hack/PATENTS`) except as otherwise noted.


## Reporting Crashes

See [Reporting Crashes](https://github.com/facebook/hhvm/wiki/Reporting-Crashes) for helpful tips on how to report crashes in an actionable manner.

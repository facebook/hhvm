# HHVM

[HHVM page](http://hhvm.com) |
[HHVM documentation](http://docs.hhvm.com/hhvm/) |
[Hacklang page](http://hacklang.org) |
[General group](https://www.facebook.com/groups/hhvm.general/) |
[Dev group](https://www.facebook.com/groups/hhvm.dev/) |
[Twitter](http://twitter.com/HipHopVM)

HHVM is an open-source virtual machine designed for executing programs written in [Hack](http://hacklang.org) and [PHP](http://php.net). HHVM uses a just-in-time (JIT) compilation approach to achieve superior performance while maintaining the development flexibility that PHP provides.

HHVM supports [Hack](http://hacklang.org), [PHP 5](http://php.net) and the major features of [PHP 7](http://hhvm.com/blog/10859/php-7-support). We are aware of [minor incompatibilities](https://github.com/facebook/hhvm/issues?q=is%3Aopen+is%3Aissue+label%3A%22php5+incompatibility%22), so please [open issues](https://github.com/facebook/hhvm/issues/new) when you find them. HHVM also supports many [extensions](http://docs.hhvm.com/hhvm/extensions/introduction) as well.

HHVM should be used together with a webserver like the built in, easy to deploy [Proxygen](http://docs.hhvm.com/hhvm/basic-usage/proxygen), or a [FastCGI](http://docs.hhvm.com/hhvm/advanced-usage/fastCGI)-based webserver on top of nginx or Apache.

## Installing

If you're new, try our [getting started guide](http://docs.hhvm.com/hhvm/getting-started/getting-started).

You can install a [prebuilt package](http://docs.hhvm.com/hhvm/installation/introduction#prebuilt-packages) or [compile from source](http://docs.hhvm.com/hhvm/installation/building-from-source).

## Running

You can run standalone programs just by passing them to hhvm: `hhvm my_script.php`.

If you want to host a website:
* Install your favorite webserver. [Proxygen](http://docs.hhvm.com/hhvm/basic-usage/proxygen) is built in to HHVM, fast and easy to deploy.
* Install our [package](http://docs.hhvm.com/hhvm/installation/introduction#prebuilt-packages)
* Start your webserver
* Run `sudo /etc/init.d/hhvm start`
* Visit your site at `http://.../index.php`

Our [getting started guide](http://docs.hhvm.com/hhvm/getting-started/getting-started) provides a slightly more detailed introduction as well as links to more information.

## Contributing

We'd love to have your help in making HHVM better. If you're interested, please read our [guide to contributing](CONTRIBUTING.md).

## License

HHVM is licensed under the PHP and Zend licenses except as otherwise noted.

The Hack typechecker (`hphp/hack`) is licensed under the BSD license (`hphp/hack/LICENSE`) with an additional grant of patent rights (`hphp/hack/PATENTS`) except as otherwise noted.

## Reporting Crashes

See [Reporting Crashes](https://github.com/facebook/hhvm/wiki/Reporting-Crashes) for helpful tips on how to report crashes in an actionable manner.

## Reporting and Fixing Security Issues

Please do not open GitHub issues or pull requests - this makes the problem
immediately visible to everyone, including malicious actors. Security issues in
HHVM can be safely reported via HHVM's Whitehat Bug Bounty program:

[https://www.facebook.com/whitehat](https://www.facebook.com/whitehat)

Facebook's security team will triage your report and determine whether or not
is it eligible for a bounty under our program.

## FAQ

Our [user FAQ](http://docs.hhvm.com/hhvm/FAQ/faq) has answers to many common questions about HHVM, from [general questions](http://docs.hhvm.com/hhvm/FAQ/faq#general) to questions geared towards those that want to [use](http://docs.hhvm.com/hhvm/FAQ/faq#users).

There is also a FAQ for [contributors](https://github.com/facebook/hhvm/wiki/FAQ#contributors) to HHVM.

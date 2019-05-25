# HHVM

[HHVM page](https://hhvm.com) |
[HHVM documentation](https://docs.hhvm.com/hhvm/) |
[Hacklang page](http://hacklang.org) |
[General group](https://www.facebook.com/groups/hhvm.general/) |
[Dev group](https://www.facebook.com/groups/hhvm.dev/) |
[Twitter](https://twitter.com/HipHopVM)

HHVM is an open-source virtual machine designed for executing programs written in [Hack](http://hacklang.org). HHVM uses a just-in-time (JIT) compilation approach to achieve superior performance while maintaining the development flexibility that PHP provides.

HHVM is intended for [Hack](http://hacklang.org) projects, and also supports a large subset of PHP 7 that is required by common tools and libraries. We no longer recommend using HHVM for purely PHP projects.

HHVM should be used together with a webserver like the built in, easy to deploy [Proxygen](https://docs.hhvm.com/hhvm/basic-usage/proxygen), or a [FastCGI](https://docs.hhvm.com/hhvm/advanced-usage/fastCGI)-based webserver on top of nginx or Apache.

## Installing

If you're new, try our [getting started guide](https://docs.hhvm.com/hhvm/getting-started/getting-started).

You can install a [prebuilt package](https://docs.hhvm.com/hhvm/installation/introduction#prebuilt-packages) or [compile from source](https://docs.hhvm.com/hhvm/installation/building-from-source).

## Running

You can run standalone programs just by passing them to hhvm: `hhvm example.hack`.

If you want to host a website:
* Install your favorite webserver. [Proxygen](https://docs.hhvm.com/hhvm/basic-usage/proxygen) is built in to HHVM, fast and easy to deploy.
* Install our [package](https://docs.hhvm.com/hhvm/installation/introduction#prebuilt-packages)
* Start your webserver
* Run `sudo /etc/init.d/hhvm start`
* Visit your site at `http://.../main.hack`

Our [getting started guide](https://docs.hhvm.com/hhvm/getting-started/getting-started) provides a slightly more detailed introduction as well as links to more information.

## Contributing

We'd love to have your help in making HHVM better. If you're interested, please read our [guide to contributing](CONTRIBUTING.md).

## License

HHVM is licensed under the PHP and Zend licenses except as otherwise noted.

The [Hack typechecker](hphp/hack) is licensed under the MIT [License](hphp/hack/LICENSE) except as otherwise noted.

## Reporting Crashes

See [Reporting Crashes](https://github.com/facebook/hhvm/wiki/Reporting-Crashes) for helpful tips on how to report crashes in an actionable manner.

## Security

For information on reporting security vulnerabilities in HHVM, see [SECURITY.md](SECURITY.md).

## FAQ

Our [user FAQ](https://docs.hhvm.com/hhvm/FAQ/faq) has answers to many common questions about HHVM, from [general questions](https://docs.hhvm.com/hhvm/FAQ/faq#general) to questions geared towards those that want to [use](https://docs.hhvm.com/hhvm/FAQ/faq#users).

There is also a FAQ for [contributors](https://github.com/facebook/hhvm/wiki/FAQ#contributors) to HHVM.

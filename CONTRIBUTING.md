# Contributing to HHVM

We'd love to have your help in making HHVM better. Before jumping into the code, please familiarize yourself with our [coding conventions](hphp/doc/coding-conventions.md). We're also working on a [Hacker's Guide to HHVM](hphp/doc/hackers-guide). It's still very incomplete, but if there's a specific topic you'd like to see addressed sooner rather than later, let us know. For documentation and any other problems, please open an [issue](http://github.com/facebook/hhvm/issues), or better yet, [fork us and send a pull request](https://github.com/facebook/hhvm/pulls). Join us on Freenode in [#hhvm](http://webchat.freenode.net/?channels=hhvm) for general discussion, or [#hhvm-dev](http://webchat.freenode.net/?channels=hhvm-dev) for development-oriented discussion.

If you want to help but don't know where to start, try fixing some of the [PHP5 tests that don't pass](hphp/test/zend/bad). You can run them with [hphp/test/run](hphp/test/run). When they work, move them to [zend/good](hphp/test/zend/good) and send a pull request.

All the open issues tagged [PHP5 incompatibility](https://github.com/facebook/hhvm/issues?labels=php5+incompatibility&page=1&state=open) are real issues reported by the community in existing PHP code and [frameworks](https://github.com/facebook/hhvm/wiki/OSS-PHP-Frameworks-Unit-Testing:-General) that could use some attention.

## Submitting Pull Requests

Before changes can be accepted a [Contributor Licensing Agreement](http://code.facebook.com/cla) must be completed. You will be prompted to accept the CLA when you submit your first pull request. If you prefer a hard copy, you can print the [pdf](https://github.com/facebook/hhvm/raw/master/hphp/doc/FB_Individual_CLA.pdf), sign it, scan it, and send it to <cla@fb.com>.

Please add appropriate test cases as you make changes; see [here](hphp/test/README.md) for more information. Travis-CI is integrated with this GitHub project and will provide test results automatically on all pull requests.

## Quick Links

 * IRC: [#hhvm](http://webchat.freenode.net/?channels=hhvm) and [#hhvm-dev](http://webchat.freenode.net/?channels=hhvm-dev) on Freenode.
 * [Issue tracker](http://github.com/facebook/hhvm/issues)

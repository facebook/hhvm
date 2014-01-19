# Contributing to HipHop

We'd love to have your help in making HipHop better. If you run into problems, please open an [issue](http://github.com/facebook/hiphop-php/issues), or better yet, fork us and send a pull request.

If you want to help but don't know where to start, try fixing some of the [Zend tests that don't pass](hphp/test/zend/bad). You can run them with [hphp/test/run](hphp/test/run). When they work, move them to [zend/good](hphp/test/zend/good) and send a pull request.

All the open issues tagged [Zend incompatibility](https://github.com/facebook/hiphop-php/issues?labels=zend+incompatibility&page=1&state=open) are real issues reported by the community in existing PHP code and [frameworks](https://github.com/facebook/hiphop-php/wiki/OSS-PHP-Frameworks-Unit-Testing:-General) that could use some attention.

## Submitting Pull Requests

Before changes can be accepted a [Contributor Licensing Agreement](http://developers.facebook.com/opensource/cla) ([pdf](https://github.com/facebook/hiphop-php/raw/master/hphp/doc/FB_Individual_CLA.pdf) - print, sign, scan, link) must be signed.

Please add appropriate test cases as you make changes; see [here](hphp/test) for more information. Travis-CI is integrated with this GitHub project and will provide test results automatically on all pulls.

## Additional Resources

 * IRC:[#hhvm on freenode](http://webchat.freenode.net/?channels=hhvm)
 * [Issue tracker](http://github.com/facebook/hiphop-php/issues)

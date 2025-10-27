# FAQ

This FAQ contains helpful hints and frequently asked questions about HHVM.

## General

### What is the history of HHVM?

For a history of what is now HHVM, please visit [our Wikipedia page](http://en.wikipedia.org/wiki/HHVM).

### How does Facebook use and run HHVM?

Facebook's entire site runs on HHVM (desktop, API and mobile), both in development and production.

### What platforms is HHVM supported on?

* **Linux**: HHVM has [Linux](/hhvm/installation/linux) support on flavors of Ubuntu and Debian.
* **Mac OS X**: HHVM has limited [MacOS](/hhvm/installation/mac) support via Homebrew.

## Users

### How do I install HHVM? Where are the binaries?

New users should read the [HHVM Introduction](/hhvm/basic-usage/introduction) guide.

### When does HHVM release a new version?

[Every two weeks](/hhvm/installation/release-schedule); each release is supported
for 6 weeks.

### Why is HHVM released every two weeks?

This allows users to choose between many small updates, or fewer, larger
updates.

### What code does HHVM currently run?

The following sites use Hack and HHVM:

* [Facebook](https://www.facebook.com)
* [Slack](https://slack.com)
* [Quizlet](https://quizlet.com)
* This website - [source is available](https://github.com/hhvm/user-documentation).

### What do I do if I run into a problem (e.g., an error, fatal or segfault)?

If you believe you may have found a security issue, please [see HHVM's SECURITY.md](https://github.com/facebook/hhvm/blob/master/SECURITY); otherwise, please [submit an issue](https://github.com/facebook/hhvm/wiki/How-to-Report-Issues).

Our user community is [on Facebook](https://facebook.com/groups/hhvm.general)
and [on Slack](https://hhvm.com/slack).

### Should I use Proxygen or FastCGI?

[Proxygen](/hhvm/basic-usage/proxygen) is strongly recommended, and used in production by Facebook.

[FastCGI](/hhvm/advanced-usage/fastCGI) is not recommended, but available for
legacy or niche use cases.

### When will HHVM support Apache or Nginx?

HHVM can be used either with `mod_proxy` and the Proxygen server, or FastCGI.

Proxygen is recommended, but care is needed to forward headers appropriately.


## Configuration and Deployment

### Why is my code slow at startup?

The HHVM JIT needs time to warm up. The warmup usually occurs somewhere on the order of 10-11 requests, at which point the JIT has performed its optimizations and off we go at peak speed.

So, in HHVM server mode, you start out by running the first couple requests in interp mode to get things primed. You don't really want to be optimizing the first few requests since that is when initialization is occurring, caches are being loaded, etc. Those code paths are generally cold later on.

After the first few requests, the JIT is on its way to optimizing.

It is advisable, but not required, if you are running an HHVM server to send the server some explicit requests that are representative of what user requests will be coming through. You can use `curl`, for example, to send these requests. This way the JIT has the information necessary to make the best optimizations for your code before any requests are actually served.

# Mac

<FbCaution>

HHVM will no longer support Homebrew going forward. For more information, see [Stopping MacOS Homebrew Support](https://hhvm.com/blog/2022/06/17/deprecating-homebrew.html).

</FbCaution>

We have limited support for installing HHVM on MacOS via [Homebrew](https://brew.sh/):

```
brew tap hhvm/hhvm
brew install hhvm
```

This will install binary packages on recent versions of MacOS (as of 2019-03-12, this means Mojave and High Sierra). If binary packages are not available
(or you pass `--build-from-source`), building will take between 20 minutes on a Mac Pro, to several hours on a MacBook Air.

Several other packages are available:

- `brew install hhvm-nightly`: installs the most recent nightly build
- `brew install hhvm-VERSION`: install a specific x.y version; for example,
  `brew install hhvm-4.56` or `brew install hhvm-4.32`

You can also [manually build from source](/docs/hhvm/installation/building-from-source).

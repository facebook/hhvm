# Linux

We support x86_64 Linux, and offer prebuilt packages on variety of Ubuntu and
Debian platforms.

While you can [build from source](/hhvm/installation/building-from-source), it is generally advisable for ease of installation and stability to use a prebuilt package.

These instructions require root; use `su -` or `sudo -i` to get a root shell first.

## Obtaining The Latest Stable Version

### Ubuntu

```
apt-get update
apt-get install software-properties-common apt-transport-https
apt-key adv --recv-keys --keyserver hkp://keyserver.ubuntu.com:80 0xB4112585D386EB94

add-apt-repository https://dl.hhvm.com/ubuntu
apt-get update
apt-get install hhvm
```

### Debian 8 Jessie, Debian 9 Stretch

```
apt-get update
apt-get install -y apt-transport-https software-properties-common
apt-key adv --recv-keys --keyserver hkp://keyserver.ubuntu.com:80 0xB4112585D386EB94

add-apt-repository https://dl.hhvm.com/debian
apt-get update
apt-get install hhvm
```

## Obtaining A Specific Release

It is generally recommended to run the newest version your code can support. This bash oneliner can be used to add the repository for the version you want to install. Replace `major` with the hhvm major version and `minor` with the hhvm minor version. You should not specify the patch part of the version number.

`apt-add-repository "deb https://dl.hhvm.com/$(lsb_release --id --short | tr '[:upper:]' '[:lower:]') $(lsb_release --codename --short)-major.minor main"`

So in order to get HHVM 4.56 you would use
`apt-add-repository "deb https://dl.hhvm.com/$(lsb_release --id --short | tr '[:upper:]' '[:lower:]') $(lsb_release --codename --short)-4.56 main""`

You will automatically receive patches such as HHVM 4.56.1, but you won't be upgraded to HHVM 4.57 and up.

If you get an HTTP 404 error from apt, please check if the hhvm version you attempted to install supports your operating system on the [blog](https://hhvm.com/blog).

A particular version of note is hhvm 3.30. It requires an extra `-lts` after the minor version number. This version is **unsupported as of November 2019** and should not be used.
`apt-add-repository "deb https://dl.hhvm.com/$(lsb_release --id --short | tr '[:upper:]' '[:lower:]') $(lsb_release --codename --short)-lts-3.30 main"`

## Choosing A Version

If you are working on a new project, you can install the [latest stable version](#obtaining-the-latest-stable-version).

If you have an existing project, you can upgrade one release at a time using the [blog](//hhvm.com/blog) to read up on breaking changes.

If you are inheriting a project and you don't know what version it was written against, check the composer.json file. This file is usually found at the root of a project (right next to .hhconfig). This file ought to include a version requirement like `"hhvm": "^4.56"`. If not, check the last time a commit was made and find what HHVM version was recent at that time using the [blog](//hhvm.com/blog).

Whatever you do, please make sure that your chosen HHVM version is receiving security updates. The [blog](//hhvm.com/blog) will inform you on what versions supported.

## Other Packages

The above commands all install the standard `hhvm` package, which is the stable, release configuration. We have a few other packages available in the repo as well:

```
# Stable debug build that is suitable for debuggers like gdb
apt-get install hhvm-dbg

# Stable developer package that contains the headers so you can create extensions, etc.
apt-get install hhvm-dev

# Nightly build (Living on the edge, rebuilt everyday, possibly unstable)
apt-get install hhvm-nightly

# Nightly debug build
apt-get install hhvm-nightly-dbg

# Nightly developer build
apt-get install hhvm-dev-nightly

```

## GPG Key Installation: Alternative Method

If you encounter issues with the `apt-key adv` command, an alternative is:

```
apt-get install -y curl
curl https://dl.hhvm.com/conf/hhvm.gpg.key | apt-key add -
apt-key finger 'opensource+hhvm@fb.com'
```

The 'fingerprint' shown by `apt-key finger` (the second line) should exactly match `0583 41C6 8FC8 DE60 17D7  75A1 B411 2585 D386 EB94`; for example:

```
$ apt-key finger 'opensource+hhvm@fb.com'
pub   rsa4096 2017-11-03 [SC]
      0583 41C6 8FC8 DE60 17D7  75A1 B411 2585 D386 EB94
uid           [ unknown] HHVM Package Signing <opensource+hhvm@fb.com>
```

If this is not the case, run `apt-key list`, then use `apt-key del` to remove any keys you don't recognize.

## Mirrors

dl.hhvm.com is fronted by a global CDN, so should be fast for all users. If you wish to maintain a local mirror, you can use AWS CLI utilities to sync:

```
aws s3 sync \
  --no-sign-request \
  --region us-west-2 \
  s3://hhvm-downloads/ \
  ./localpath/ \
  --exclude '*index.html'
```

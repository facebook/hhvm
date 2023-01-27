This website is itself written in Hack and running on HHVM. You can
see the current [HHVM version used on the deployed site](https://github.com/hhvm/user-documentation/blob/main/.deploy/built-site.Dockerfile#L3).

## Prerequisites

You'll need HHVM
(see [installation instructions](/hhvm/installation/introduction))
installed on your local machine, and the version must have the same major
and minor (e.g. `4.123`) as the version specification in the `composer.json`
file.

You'll also need a checkout of the source code for this site.

```
$ git clone git@github.com:hhvm/user-documentation.git
```

## Composer Dependencies

We use Composer to manage our PHP library dependencies and to autoload classes.
Composer is written in PHP, so you need PHP installed.

```
# Ubuntu example, your environment may differ.
$ apt-get install php-cli
```

You can now follow the instructions on [the Composer website](https://getcomposer.org/) to install it.

Once you've installed `composer.phar`, you can install the website dependencies into `vendor/` and create the autoload map.

```
$ cd user-documentation
user-documentation$ php /path/to/composer.phar install
```

### Updating Dependencies

We require that this whole repository has no type errors.

```
$ hh_client
No errors!
```

If you are seeing errors in `vendor/`, re-run the composer install command to ensure that all dependencies are up to date.

**NOTE**: If you add, delete or rename a class in the primary source tree `src/`, you should run `php composer.phar dump-autoload` in order to make the autoload maps refresh correctly; otherwise you will get class not found exceptions.

## Running A Local Instance

Generate `public/` by running the build script. This script will only
run the steps related to your changes, so the first run will be slower
than later runs.

```
$ hhvm bin/build.php
```

HHVM has a built-in webserver, so it can serve the entire website.

```
$ cd public
$ hhvm -m server -p 8080 -c ../hhvm.dev.ini
```

You can now see your local instance by visiting <http://localhost:8080>.

When you make changes, you'll need to run `build.php` again. You can
leave HHVM serving the site, and it will pick up the changes
automatically.

## Running An Old Instance

If you want to see old versions of the docs, we provide [regular
Docker images of this
site](https://hub.docker.com/r/hhvm/user-documentation/tags). This is
not suitable for development, but it's useful if you're working with
an old HHVM/Hack version.

1. Install [Docker](https://docs.docker.com/engine/installation/).
2. Start a container with `docker run -p 8080:80 -d hhvm/user-documentation`.
3. You can then access a local copy of the documentation at <http://localhost:8080>.

## Running A Production Instance

Running a production instance is very similar to a development
instance.

Configure a webserver and HHVM to serve the `public/`
directory. Ensure that all requests that don't match a local file are
served by `index.php`.

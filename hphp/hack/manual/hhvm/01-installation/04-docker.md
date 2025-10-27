# Docker

We publish Docker images to Docker Hub.  These can be used to install HHVM in a containerized environment.  If you are new to Docker follow their [getting started guide](https://docs.docker.com/engine/getstarted/) to learn more.  All of our images are available [here](https://hub.docker.com/u/hhvm/) (including one for this doc site).  To get you started, here are a couple examples:

## Running HHVM Scripts

```
docker pull hhvm/hhvm
docker run --tty --interactive hhvm/hhvm:latest /bin/bash -l
hhvm --version
```

## Building a Docker Image for a Website

Start by creating the following files and folders in a directory:

*`Dockerfile`*

```
FROM hhvm/hhvm-proxygen:latest

RUN rm -rf /var/www
ADD . /var/www

EXPOSE 80
```

*`public/index.php`*

```
<?hh

<<__EntryPoint>>
function main(): void {
  echo "Hello World!\n";
}
```

Now in a shell run:

```
docker build .
docker run -p 0.0.0.0:80:80 <Replace With The Hash Identifying The Build>
```

You should now have a running web server hosting the *`index.php`* script visit http://localhost/ to check it out.  To shut it down run:

```
docker ps
docker stop <The CONTAINER ID shown in the output from docker ps>
```

Checkout the setup for this docsite on [github](https://github.com/hhvm/user-documentation) to see how this might scale.

### Best Practices

The `hhvm/hhvm-proxygen` image serves `/var/www/public`, and defaults to `/var/www/public/index.php` for `/` and pages that don't exist. We **strongly** recommend only putting files that need to be directly accessible by external users in this directory - this usually means just `index.php` and static resources such as CSS, JavaScript, and images. The rest of your code can reside elsewhere in the image. It's fairly common for `public/` to be a subdirectory of your project root - in which case, you can install your entire project to `/var/www` in the container, which is what we do above. For example, most of the source for this website is in `src/`, but we have [a single `index.php` in `public/`](https://github.com/hhvm/user-documentation/blob/master/public/index.php) which initializes the stack using the code from `src/` and `vendor/`.

This avoids issues such as:
 - exposing configuration files, which might contain credentials such as database passwords
 - accidentally exposing the source (with history) [via .git or .hg directories](http://www.jamiembrown.com/blog/one-in-every-600-websites-has-git-exposed/)
 - exposing scripts that shouldn't be executable remotely, e.g. the contents of your projects' `bin/` directory
 - any of the above from your recursive dependencies in `vendor/` even if your application code is safe

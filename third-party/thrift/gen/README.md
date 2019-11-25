# Update generated thrift files

From #2ddd15bb621e40deee8a854c4670c4392ed49828

To do this:

- install docker (`apt-get install docker.io`)
- add yourself to the docker group if on Ubuntu
- run the stuff below:

```
$ os_image=ubuntu:16.04 gcc_version=5 make_parallelism=16 travis_cache_dir=~/travis_ccache \
  build/fbcode_builder/travis_docker_build.sh`
$ docker ps -a # to get the container ID
$ export CONTAINER_ID= # whatever you got from docker ps -a
$ cd third-party/thrift/gen/thrift/lib
$ rm -rf cpp2 thrift
$ mkdir cpp2 thrift
$ # Don't use /home/install/... as those are only the .h files, and we need the .cpp too
$ docker cp $CONTAINER_ID:/home/fbthrift/thrift/thrift/lib/thrift/gen-cpp2/ thrift/gen-cpp2
$ docker cp $CONTAINER_ID:/home/fbthrift/thrift/thrift/lib/cpp2/ cpp2/
```

Assuming you're rebuilding and retesting, re-run cmake so that the glob patterns used to
create makefiles are re-evaluated.

If you need to poke around the docker image to debug or find stuff:

```
$ docker commit $CONTAINER_ID snapshot_name # create an image based on $CONTAINER_ID
$ docker run -it snapshot_name /bin/bash -l # start bash in a new container, based on the filesystem from the other container
```

Next, follow the instructions for third-party/mcrouter/gen/

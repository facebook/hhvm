1. Follow the instructions in third-party/thrift/gen/thrift/lib/README.md to get
   a docker container containing a working fbthrift compiler
2. run `docker commit $CONTAINER_ID` and save the hash; this gets you a new docker
   image that can be used to run commands with that thrift compiler. Keep that hash
   as `$IMAGE_ID`
3. run commands below:

```
$ cd third-party/mcrouter/src/mcrouter/lib/carbon
$ mkdir -p ../../../../gen/mcrouter/lib/carbon
$ docker run --rm -w $(pwd) -v $(pwd):$(pwd) -v $(realpath ../../../../gen/mcrouter/lib/carbon):$(pwd)/outdir $IMAGE_ID /home/install/bin/thrift1 -gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/carbon/ -o outdir carbon.thrift
$ docker run --rm -w $(pwd) -v $(pwd):$(pwd) -v $(realpath ../../../../gen/mcrouter/lib/carbon):$(pwd)/outdir $IMAGE_ID /home/install/bin/thrift1 -gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/carbon/ -o outdir carbon_result.thrift
```

```
$ cd third-party/mcrouter/src/mcrouter/lib/network/gen
$ mkdir -p ../../../../../gen/mcrouter/lib/network/gen 
$ docker run --rm -w $(pwd) -v $(realpath ../../../../):$(realpath ../../../../) -v $(realpath ../../../../../gen/mcrouter/lib/network/gen):$(pwd)/outdir $IMAGE_ID /home/install/bin/thrift1 -gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/network/gen/ -o outdir Common.thrift
$ docker run --rm -w $(pwd) -v $(realpath ../../../../):$(realpath ../../../../) -v $(realpath ../../../../../gen/mcrouter/lib/network/gen):$(pwd)/outdir $IMAGE_ID /home/install/bin/thrift1 -gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/network/gen/ -o outdir Memcache.thrift
$ docker run --rm -w $(pwd) -v $(realpath ../../../../):$(realpath ../../../../) -v $(realpath ../../../../../gen/mcrouter/lib/network/gen):$(pwd)/outdir $IMAGE_ID /home/install/bin/thrift1 -gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/network/gen/ -o outdir MemcacheService.thrift
```

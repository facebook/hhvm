1. Follow the instructions in third-party/thrift/gen/thrift/lib/README.md to get
   a working fbthrift compiler, and set the `THRIFTC` variable as described there
   as `$IMAGE_ID`
2. run the commands below:

```
cd third-party/mcrouter/gen
rm -rf mcrouter
mkdir -p mcrouter/lib/carbon
SRC_DIR=../src/mcrouter/lib/carbon
for FILE in carbon.thrift carbon_result.thrift; do
  $THRIFTC --gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/carbon/ \
    -o mcrouter/lib/carbon -I $SRC_DIR \
    $SRC_DIR/$FILE
done
mkdir -p mcrouter/lib/network/gen
for FILE in Common.thrift Memcache.thrift MemcacheService.thrift; do
  $THRIFTC \
    --gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/network/gen/ \
    -o mcrouter/lib/network/gen \
    -I $SRC_DIR -I $SRC_DIR/../../../.. $SRC_DIR/$FILE
done
git status # Check the results look reasonable
```

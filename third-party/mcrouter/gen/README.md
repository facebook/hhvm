1. Follow the instructions in third-party/thrift/gen/thrift/lib/README.md to get
   a working fbthrift compiler, and set the `THRIFTC` variable as described there
   as `$IMAGE_ID`
2. run the commands below:

```
cd third-party/mcrouter/src
SRC_DIR=mcrouter/lib/carbon
OUT_DIR=$(pwd)/../gen/$SRC_DIR
rm -rf $OUT_DIR
mkdir -p $OUT_DIR
cd $SRC_DIR
for FILE in carbon.thrift carbon_result.thrift; do
  $THRIFTC --gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/carbon/ \
    -o $OUT_DIR -I $SRC_DIR \
    $FILE
done

cd ../../..
SRC_DIR=mcrouter/lib/network/gen
OUT_DIR=$(pwd)/../gen/$SRC_DIR
rm -rf $OUT_DIR
mkdir -p $OUT_DIR
cd $SRC_DIR
for FILE in Common.thrift Memcache.thrift MemcacheService.thrift; do
  $THRIFTC \
    --gen mstch_cpp2:stack_arguments,include_prefix=mcrouter/lib/network/gen/ \
    -o $OUT_DIR -I . -I ../../../.. \
    $FILE
done
git status # Check the results look reasonable
```

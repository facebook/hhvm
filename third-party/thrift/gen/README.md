# Fully build Thrift

The easiest way to do this is to checkout `fbthrift`, then run:

```
build/fbcode_builder/getdeps.py \
  --scratch-path=/tmp/scratch \
  --allow-system-paths \
  build fbthrift
```

If you are on MacOS, you will need to use standard Apple XCode, not a
specially-packaged version:
- `xcrun --show-sdk-path` will show which XCode and SDK is being used
- `sudo xcode-select --switch PATH` can be used to change to a different XCode

# Using the thrift compiler to update the codegen

The thrift compiler is `installed/fbthrift/bin/thrift1` inside the scratch
patch you specified.


```
THRIFTC=/tmp/scratch/installed/fbthrift/bin/thriftq
cd third-party/thrift/src
SRC_DIR=thrift/lib/thrift
OUT_DIR=$(pwd)/../gen/$SRC_DIR
rm -rf $OUT_DIR
mkdir -p $OUT_DIR
cd $SRC_DIR
$THRIFTC --gen mstch_cpp2:templates,no_metadata -I $SRC_DIR \
  -o $OUT_DIR reflection.thrift
$THRIFTC --gen mstch_cpp2 -I $SRC_DIR \
  -o $OUT_DIR metadata.thrift
$THRIFTC --gen mstch_cpp2 -I $SRC_DIR \
  -o $OUT_DIR frozen.thrift
$THRIFTC --gen mstch_cpp2:json,no_metadata -I $SRC_DIR \
  -o $OUT_DIR RpcMetadata.thrift
$THRIFTC --gen mstch_cpp2:no_metadata -I $SRC_DIR \
  -o $OUT_DIR RocketUpgrade.thrift
git status # Check the results look reasonable
```

These commands are based on the arguments to `thrift_generate()` in
`thrift/lib/thrift/CMakeLists.txt` inside the fbthrift repository.

# Next Steps

Assuming you're rebuilding and retesting, re-run cmake so that the glob patterns used to
create makefiles are re-evaluated.

Next, follow the instructions for third-party/mcrouter/gen/

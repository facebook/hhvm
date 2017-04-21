## Generate the section ordering file for gold using oss-performance workloads
This file details the process of generating the hot section ordering file used by the gold linker.
The first step is to collect data using perf (hf-prod-collect.sh). The data is used by the hfsort tool which generates the file hotfuncs.txt.

Prerequisites
- [oss-performance](https://github.com/hhvm/oss-performance)
- HHVM sources


### Configure, build and install HHVM 
Go to the directory where the HHVM sources are.
HHVM must be installed with debug symbols.
HHVM_DIR = the install directory of HHVM

``` bash
cd hhvm_src
./configure -DCMAKE_INSTALL_PREFIX=$HHVM_DIR -DCMAKE_BUILD_TYPE=RelWithDebInfo
make
sudo make install
```


### Allow the collection of data using perf
``` bash
echo "-1" | sudo tee /proc/sys/kernel/perf_event_paranoid
```

### Collect data using oss-performance and hf-prod-collect.sh
The hf-prod-collect script uses perf to collect data. The script creates a nm file (hhvm.nm) with HHVM symbols and a zip file containing the perf data (perf.pds.gz). Both these files are located in the directory /tmp/hp-prof.

In this example we use the WordPress target to generate the file. The collection starts after the warmup is completed and it takes 60 seconds (equal to the time the benchmark runs). 

``` bash 
cd hphp/tools/hfsort

/usr/local/hhvm/HHVM-3.8.0/bin/hhvm ${OSS_PERFORMANCE_PATH}/perf.php \
                --wordpress  \
                --i-am-not-benchmarking \
                --profBC \
                --hhvm-extra-arguments -vEval.KeepPerfPidMap=true \
                --exec-after-warmup="SLEEP_TIME=60 HHVM_BIN_PATH=${HHVM_BIN_PATH} hf-prod-collect.sh > dump_hfprod.txt 2>&1 &" \
                --hhvm=${HHVM_BIN_PATH}

```

### Generate the hot section ordering by using the default algorithm
Running hfsort will generate the file hotfuncs.txt in the working directory.

``` bash
hfsort /tmp/hf-prof/hhvm.nm /tmp/hf-prof/perf.pds.gz
```

### Use a custom file for function ordering when linking HHVM with gold
When calling configure, you can specify which file to use at link time (SECTION_ORDERING_FILE).
``` bash
./configure -DCMAKE_INSTALL_PREFIX=$HHVM_DIR -DSECTION_ORDERING_FILE=$FILE_PATH/hotfuncs.txt
```

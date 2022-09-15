#!/bin/bash
#
# Copyright (c) 2005-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
#
#

# Script used to generate tbbvars.[c]sh scripts
bin_dir="$PWD"  # 
cd "$tbb_root"  # keep this comments here
tbb_root="$PWD" # to make it unsensible
cd "$bin_dir"   # to EOL encoding
cat >./tbbvars.sh <<EOF
#!/bin/bash
export TBBROOT="${tbb_root}" #
tbb_bin="${bin_dir}" #
if [ -z "\$CPATH" ]; then #
    export CPATH="\${TBBROOT}/include" #
else #
    export CPATH="\${TBBROOT}/include:\$CPATH" #
fi #
if [ -z "\$${2}LIBRARY_PATH" ]; then #
    export ${2}LIBRARY_PATH="\${tbb_bin}" #
else #
    export ${2}LIBRARY_PATH="\${tbb_bin}:\$${2}LIBRARY_PATH" #
fi #
if [ -z "\$${1}LD_LIBRARY_PATH" ]; then #
    export ${1}LD_LIBRARY_PATH="\${tbb_bin}" #
else #
    export ${1}LD_LIBRARY_PATH="\${tbb_bin}:\$${1}LD_LIBRARY_PATH" #
fi #
${TBB_CUSTOM_VARS_SH} #
EOF
cat >./tbbvars.csh <<EOF
#!/bin/csh
setenv TBBROOT "${tbb_root}" #
setenv tbb_bin "${bin_dir}" #
if (! \$?CPATH) then #
    setenv CPATH "\${TBBROOT}/include" #
else #
    setenv CPATH "\${TBBROOT}/include:\$CPATH" #
endif #
if (! \$?${2}LIBRARY_PATH) then #
    setenv ${2}LIBRARY_PATH "\${tbb_bin}" #
else #
    setenv ${2}LIBRARY_PATH "\${tbb_bin}:\$${2}LIBRARY_PATH" #
endif #
if (! \$?${1}LD_LIBRARY_PATH) then #
    setenv ${1}LD_LIBRARY_PATH "\${tbb_bin}" #
else #
    setenv ${1}LD_LIBRARY_PATH "\${tbb_bin}:\$${1}LD_LIBRARY_PATH" #
endif #
${TBB_CUSTOM_VARS_CSH} #
EOF
# Workaround for copying Android* specific stl shared library to "."
if [ ! -z "${LIB_STL_ANDROID}" ]; then #
	cp ${LIB_STL_ANDROID} . #
fi #

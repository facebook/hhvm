#!/bin/bash
# prevent buck from downloading a new version, ever
BUCKVERSION="$(cat .buckversion)"; export BUCKVERSION

# build the parse_www_job and hh_single_compile binaries
buck build \
    //hphp/hack/src:hh_single_compile \
    //hphp/hack/scripts/justparse:parse_www_job \
    @mode/opt

# grab the parse_www_job and hh_single_compile binaries from
# the output directory and put them in the cwd with a SANDCASTLE
# suffix
# in the next step, they will be picked up by everstore
cp "$(find "$(buck root)"/buck-out/opt/gen/hphp -name hh_single_compile.opt -print -quit)" \
    ./hh_single_compile.opt.SANDCASTLE
cp "$(find "$(buck root)"/buck-out/opt/gen/hphp -name parse_www_job.par -print -quit)" \
    ./parse_www_job.par.SANDCASTLE

#!/bin/sh

cd ${HPHP_HOME}
# Generate src/runtime/vm/repo_schema.h
REPO_TREE=`git log -n1 --pretty=format:%T HEAD`
if [ -z "${REPO_TREE}" ]; then
  # HipHop has been divorced from git,
  # define REPO_SCHEMA as current time
  # to play it safe
  HHVM_REPO_SCHEMA=`date +%s%N`
else
  REPO_MODS=`git diff --name-only HEAD`
  if [ "${REPO_MODS}" != "" ]; then
    REPO_TREE=`
    export GIT_INDEX_FILE=.git-index-$$$$;
    git read-tree ${REPO_TREE};
    git update-index --add --remove ${REPO_MODS};
    git write-tree;
    rm -f $GIT_INDEX_FILE`
  fi
  HHVM_REPO_SCHEMA=`git ls-tree --full-tree ${REPO_TREE} src/ | grep -v src/test | git hash-object --stdin`
fi

echo "#define REPO_SCHEMA \"${HHVM_REPO_SCHEMA}\"" > src/runtime/vm/repo_schema.h

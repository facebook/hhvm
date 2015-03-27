# Copy this file to your shell init script to get tab completion :-)

_benchy_complete()
{
  COMPREPLY=()
  CUR=${COMP_WORDS[COMP_CWORD]}
  PREV_CWORD=`expr $COMP_CWORD - 1`
  if [ "${COMP_WORDS[PREV_CWORD]}" = ":" ]
  then
    # On the path part.
    RESULTS=($(compgen -d -f -o bashdefault -- "${CUR}"))
    for RESULT in ${RESULTS[@]}
    do
      COMPREPLY+=(${RESULT})
    done
  else
    # On the branch part.
    if OUTPUT=$(git branch 2>&1); then
      BRANCHES=`echo "$OUTPUT" | awk -F ' +' '! /\(no branch\)/ {print $2}' | grep "^$CUR"`
    elif OUTPUT=$(hg bookmark 2>&1); then
      BRANCHES=`echo "$OUTPUT" | rev | awk -F ' +' '{print $2}' | rev | grep "^$CUR"`
    else
      BRANCHES=""
      ! true
    fi

    if [ $? -eq 0 ]
    then
      for BRANCH in $BRANCHES
      do
        COMPREPLY+=($BRANCH)
      done
    fi
  fi
}

complete -o nospace -o filenames -F _benchy_complete benchy.py

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
    BRANCHES=`git branch | awk -F ' +' '! /\(no branch\)/ {print $2}' | grep "^$CUR"`
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

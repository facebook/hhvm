# Copy this file to your shell init script to get tab completion :-)

_benchy_complete()                                                                 
{                                                                                  
  CUR=${COMP_WORDS[COMP_CWORD]}                                                    
  COMPREPLY=()                                                                     
  BRANCHES=`git branch | awk -F ' +' '! /\(no branch\)/ {print $2}' | grep "^$CUR"`
  if [ $? -eq 0 ]                                                                  
  then                                                                             
    for BRANCH in $BRANCHES                                                        
    do                                                                             
      COMPREPLY+=($BRANCH)                                                         
    done                                                                           
  fi                                                                               
}                                                                                  
                                                                                   
complete -F _benchy_complete benchy.py

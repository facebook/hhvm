// Copyright (C) 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/* Copyright (C) 2012 IBM Corporation and Others. All Rights Reserved */

#include <stdio.h>
#include <demangle.h>

void showSym(char *str) {
  char *rest;
  struct Name *name = Demangle(str, rest); // "f__1XFi"

  printf("# '%s'\n", str);
  if(*rest) printf("\trest: '%s'\n", rest);
  if(name->Kind() == MemberFunction) {
    //((MemberFunctionName *) name)->Scope()->Text() is "X"
    //((MemberFunctionName *) name)->RootName() is "f"
    //((MemberFunctionName *) name)->Text() is "X::f(int)"
    printf("\t=> %s\n", ((MemberFunctionName *) name)->Text());
  } else {
    printf("\t(not MemberFunction)\n");
  }
}





int main(int argc, /*const*/ char *argv[]) {
  if(argc>1) {
    for(int i=1;i<argc;i++) {
       showSym(argv[i]);
    }
  } else {
    printf("Usage: %s <symbol> ...\n", argv[0]);
  }



}

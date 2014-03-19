" Copyright (c) 2014, Facebook, Inc.
" All rights reserved.
"
" This source code is licensed under the BSD-style license found in the
" LICENSE file in the "hack" directory of this source tree. An additional grant
" of patent rights can be found in the PATENTS file in the same directory.

" Vim syntax file for Hack
"
" Language: HackForHipHop (PHP)
"
" Facebook/XHP classes and functions
syn keyword phpFunctions param_post param_get contained
syn keyword phpFunctions wait wait_for wait_forv wait_forva result wait_for_result contained

syn keyword phpType void mixed tuple contained
syn keyword phpDefine attribute category children required enum contained
syn keyword phpSCKeyword async contained
syn keyword phpSpecial await contained

""""""""""""""""""""""""""""""""" " HACK " """"""""""""""""""""""""""""""""

syn region hackGenericType contained matchgroup=hackGenericType
      \ start="\w\+<"hs=e
      \ end=">"
      \ contains=hackGenericType2

syn region hackGenericType2 contained  matchgroup=hackGenericType2
      \ start="\w\+<"hs=e
      \ end=">"
      \ contains=hackGenericType3

syn region hackGenericType3 contained  matchgroup=hackGenericType3
      \ start="\w\+<"hs=e
      \ end=">"
      \ contains=hackGenericType

syn region phpRegion matchgroup=Delimiter keepend
      \ start="<?hh\( // partial\| // strict\| // decl\|\s*\)\(\s\|$\)"
      \ end="?>"
      \ contains=@phpClTop

" This messes up things, because ArrayAccess<T> would render ArrayAccess as a
" keyword and then <T> will become XHP. If you figure out how to fix this,
" LMK please.
syn cluster phpClConst remove=phpClasses,hackGenericType
syn cluster phpClInside add=hackGenericType

hi def link hackGenericType Type
hi def link hackGenericType2 Keyword
hi def link hackGenericType3 Special

" Copyright (c) 2014, Facebook, Inc.
" All rights reserved.
"
" This source code is licensed under the BSD-style license found in the
" LICENSE file in the "hack" directory of this source tree. An additional grant
" of patent rights can be found in the PATENTS file in the same directory.

"
" HackForHipHop Checker v1.2 for Vim
"

if !exists("g:fb_hack_autoclose")
  let g:fb_hack_autoclose = 1
endif

if !exists("g:fb_hack_on")
  " Set this to 0 if you don't like the autocommand on write
  let g:fb_hack_on = 1
endif

if !exists("g:fb_hack_snips")
  " Set this to 0 if you don't like the hhs/hhp insert snippets
  let g:fb_hack_snips = 1
endif

function! s:Exec(cmd)
  let ret = system(a:cmd)
  return strpart(ret, 0, strlen(ret) - 1)
endfunction

function! s:CheckHack()
  let old_make = &makeprg
  let old_fmt = &errorformat
  let temp = s:Exec('mktemp')
  call system('hh_client --from-vim | sed "s/No errors!//" > '.temp)
  let &errorformat = '%EFile "%f"\, line %l\, characters %c-%.%#,'
  let &errorformat.= '%Z%m,'
  let &errorformat.= 'Error: %m,'
  let &errorformat.= '%m,'
  execute 'cgetfile '.temp
  if g:fb_hack_autoclose == 1
    botright cwindow
  else
    botright copen
  endif

  let &makeprg = old_make
  let &errorformat = old_fmt
endfunction

function! s:FindRefs(fn)
  let old_make = &makeprg
  let old_fmt = &errorformat
  let temp = s:Exec('mktemp')
  call system('hh_client --from-vim --find-refs '.a:fn.' > '.temp)
  let &errorformat = '%EFile "%f"\, line %l\, characters %c-%.%#,'
  let &errorformat.= '%Z%m,'
  let &errorformat.= 'Error: %m,'
  let &errorformat.= '%m,'
  execute 'cgetfile '.temp
  if g:fb_hack_autoclose == 1
    botright cwindow
  else
    botright copen
  endif

  let &makeprg = old_make
  let &errorformat = old_fmt
endfunction

function! s:CheckHackAutocommand()
  if g:fb_hack_on == "1"
    " TODO: Add a small delay to make sure Hack server gets notified of the change.
    call s:CheckHack()
    redraw!
  endif
endfunction

function! s:CheckHackCommand()
  call s:CheckHack()
  redraw!
endfunction

function! s:HackToggleAutocommand()
  if g:fb_hack_on == "1"
      let g:fb_hack_on = "0"
  else
      let g:fb_hack_on = "1"
  endif
endfunction

function! s:HackToggleAutoclose()
  if g:fb_hack_autoclose == 1
    let g:fb_hack_autoclose = 0
  else
    let g:fb_hack_autoclose = 1
  endif
endfunction

function! s:HackTypeCW()
  let cmd='hh_client --type-at-pos '.fnameescape(expand('%')).':'.line('.').':'.col('.')
  let output='HackType: '.system(cmd)
  let output=substitute(output, '\n$', '', '')
  echo output
endfunction

if g:fb_hack_snips == 1
  iabbrev hhs <?hh // strict
\<CR>// Copyright 2004-present Facebook. All Rights Reserved.
  iabbrev hhp <?hh
\<CR>// Copyright 2004-present Facebook. All Rights Reserved.
  iabbrev hhd <?hh // decl
\<CR>// Copyright 2004-present Facebook. All Rights Reserved.
endif

command! HackMake call s:CheckHackCommand()
command! HackToggle call s:HackToggleAutocommand()
command! HackToggleAutoclose call s:HackToggleAutoclose()
command! HackType call s:HackTypeCW()
command! -nargs=1 HackFindRefs call s:FindRefs(<q-args>)

" Using [; and ]; to switch between quickfix windows
noremap [; :colder<CR>
noremap ]; :cnewer<CR>

au BufWritePost *.php call s:CheckHackAutocommand()
au BufWritePost *.hhi call s:CheckHackAutocommand()
au BufRead,BufNewFile *.hhi set filetype=php

" Keep quickfix window at an adjusted height.
au FileType qf call AdjustWindowHeight(3, 10)
function! AdjustWindowHeight(minheight, maxheight)
  exe max([min([line("$"), a:maxheight]), a:minheight]) . "wincmd _"
endfunction

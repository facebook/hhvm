" Copyright (c) 2014, Facebook, Inc.
" All rights reserved.
"
" This source code is licensed under the BSD-style license found in the
" LICENSE file in the "hack" directory of this source tree. An additional grant
" of patent rights can be found in the PATENTS file in the same directory.

" HackForHipHop Omni Complete for Vim
" 
" Delegates all to hackomni.py script

if !exists("g:fb_hack_omnicomplete")
  " set this to 0 in your vimrc if you'd like to disable the 
  " auto activation of Hack's omni completion settings
  let g:fb_hack_omnicomplete = 1
endif

let s:vim_script_filename=expand("<sfile>")

fun! HackOmniComplete(findstart, base)
python << endpython
# 1. import
import vim, os, sys
sys.path.append(os.path.abspath(os.path.dirname(vim.eval('s:vim_script_filename'))))
import hackomni

# 2. delegate
hackomni.HackOmniComplete(bool(int(vim.eval('a:findstart'))), vim.eval('a:base')).main()

endpython
endfun

if g:fb_hack_omnicomplete == 1

  autocmd FileType php set omnifunc=HackOmniComplete
  " Use CTRL-Space to run autocomplete in gVim and other nonconsole vim GUIs
  autocmd FileType php imap <C-space> <C-x><C-o>
  autocmd FileType php imap <A-space> <C-x><C-o>

endif

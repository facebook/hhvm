" Copyright (c) 2014, Facebook, Inc.
" All rights reserved.
"
" This source code is licensed under the BSD-style license found in the
" LICENSE file in the "hack" directory of this source tree. An additional grant
" of patent rights can be found in the PATENTS file in the same directory.

" HackForHipHop Vim-ColorMode v0.2
"
" Delegates most of the work to the hackcolor.py script

if !exists("g:fb_hack_colormode")
  " set this to 0 in your vimrc if you'd like to disable this plugin
  let g:fb_hack_colormode = 1
endif

let s:vim_script_filename=expand("<sfile>")

fun! HackColorMode(path, mode)
    if &modified
      echo "Please save the file first."
      return
    endif
    if a:mode==1
      :split
    elseif a:mode==2
      :vsplit
    endif
    setl bt=nofile
    highlight HackUnsafe ctermbg=Red ctermfg=White term=underline gui=bold,underline guifg=red

python << endpython
import vim, os, sys
sys.path.append(os.path.abspath(os.path.dirname(vim.eval('s:vim_script_filename'))))

import hackcolor
hackcolor.HackColorMode(vim.eval('a:path')).main()
endpython
endfun

command! HackColor call HackColorMode(expand('%'), 1)
command! HackColorv call HackColorMode(expand('%'), 2)

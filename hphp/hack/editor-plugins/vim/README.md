Installation
==============
If you do not have a preferred installation method, I recommend installing
[pathogen.vim](https://github.com/tpope/vim-pathogen), and then simply
copy and paste:

 cd ~/.vim/bundle && ln -s /usr/share/hhvm/hack/vim hack-vim

(If you are using the source distribution instead of the binary distribution,
adjust the path to the `vim` directory as needed.)

Once help tags have been generated (using the Pathogen :Helptags command),
you can read the manual with `:help hack`.

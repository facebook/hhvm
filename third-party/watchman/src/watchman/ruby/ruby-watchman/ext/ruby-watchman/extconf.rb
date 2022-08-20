# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

require 'mkmf'

def header(item)
  unless find_header(item)
    puts "couldn't find #{item} (required)"
    exit 1
  end
end

# mandatory headers
header('ruby.h')
header('fcntl.h')
header('sys/errno.h')
header('sys/socket.h')

# variable headers
have_header('ruby/st.h') # >= 1.9; sets HAVE_RUBY_ST_H
have_header('st.h')      # 1.8; sets HAVE_ST_H

RbConfig::MAKEFILE_CONFIG['CC'] = ENV['CC'] if ENV['CC']

create_makefile('ruby-watchman/ext')

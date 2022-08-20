# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

def debug_tokenizer?
  false
end
def debug_conditions?
  false
end

DELIMS = {
  comment:  "<%!",
  cond:     "<%#",
  inv_cond: "<%^",
  end_cond: "<%/",
  echo:     "<%",
  close:    "%>",
}

if ARGV.length != 1
  $stderr.puts "usage: #{__FILE__} file.mustache"
  exit -1
end
file = File.read(ARGV.first)
puts "read file #{ARGV.first}"

class Tokenizer
  attr_reader :source
  def initialize(source)
    @source = source
  end

  Tok = Struct.new(:value, :type, :line, :col) do
    def to_s
      longest = DELIMS.keys.map(&:to_s).map(&:length).max
      "Token #{type.to_s.ljust(longest)} @ {#{to_loc_s(2)}} -> #{value.dump}"
    end
    def to_loc_s(numjust = 0)
      "l:#{line.to_s.rjust(numjust)}, c:#{col.to_s.rjust(numjust)}"
    end
    def to_loc_and_val_s
      "#{value.dump} (#{to_loc_s})"
    end
  end

  def tokenize
    source = @source
    toks = []

    curr_line = 1
    curr_col = 1
    chew_up_to = ->(position, is_delim) {
      value = source[0 ... position]
      type = if !is_delim
        :source
      else
        DELIMS.find do |k, v|
          v == value
        end.first
      end

      tok = Tok.new(value, type, curr_line, curr_col)
      toks << tok
      num_nls = tok.value.count("\n")
      if num_nls == 0
        curr_col += tok.value.length
      else
        curr_col = (tok.value.split("\n").last || "").length + 1
      end
      curr_line += num_nls
      source = source[position .. -1]
    }

    until source == ""
      next_delim = DELIMS.values.map do |delim|
        if idx = source.index(delim)
          {pos: idx, delim: delim}
        end
      end.select do |a|
        a != nil
      end

      # no more tokens, consume rest of source
      if next_delim.empty?
        chew_up_to.call(source.length, false)
        next
      end

      # find the next delimiter
      next_delim = next_delim.reduce do |a, b|
        if a[:pos] == b[:pos]
          a[:delim].length > b[:delim].length ? a : b
        else
          a[:pos] < b[:pos] ? a : b
        end
      end

      # if it's not at the beginning of the string, consme
      # source up to that point
      if next_delim[:pos] != 0
        chew_up_to.call(next_delim[:pos], false)
      else
        # delim is at beginning of string
        chew_up_to.call(next_delim[:delim].length, true)
      end
    end

    toks
  end
end

tokenizer = Tokenizer.new(file)
toks = tokenizer.tokenize

if debug_tokenizer?
  puts "tokenizer delims: #{tokenizer.delims}"
  puts "tokens in source: "
  toks.each do |tok|
    puts " -> #{tok.to_s}"
  end
end
puts "done tokenizing (#{toks.length} tokens)."

# joining all the tokens back together should yield the exact same source back
if file != toks.map(&:value).join("")
  puts "invalid tokenization: "
  puts " toks -> " + toks.map(&:value).join("").dump
  puts " file -> " + file.dump
else
  puts "tokenization passed sanity check."
end

class Parser
  def initialize(tokens)
    @tokens = tokens
  end

  def validate!
    tokens = @tokens

    cond_stack = []

    while tokens.any?
      tok = tokens.shift
      case tok.type
      when :comment then
        while true
          src_or_close = tokens.shift
          unless [:source, :close].include?(src_or_close.type)
            raise(
              "expected closing delim or non-delimieter in "+
              "comment starting with #{tok.to_loc_and_val_s}"
            )
          end
          if src_or_close.type == :close
            break
          end
        end
      when :echo, :cond, :inv_cond, :end_cond then
        ident = tokens.shift
        unless ident
          raise(
            "expected identifier after opening delim " +
            tok.to_loc_and_val_s
          )
        end
        if !ident || ident.type != :source
          raise(
            "expected identifier after #{tok.type} token, "+
            "got #{ident.to_loc_and_val_s}"
          )
        end
        close = tokens.shift
        unless close
          raise(
            "expected closing delim after #{ident.to_loc_and_val_s}"
          )
        end
        if close.type != :close
          raise(
            "expected closing delim after ident token, "+
            "got #{close.to_loc_and_val_s}"
          )
        end

        if [:cond, :inv_cond].include?(tok.type)
          puts "opening condition/scope -> #{ident}" if debug_conditions?
          cond_stack.push ident
        end

        if :end_cond == tok.type
          puts "closing condition/scope -> #{ident}" if debug_conditions?
          opening_cond = cond_stack.pop
          if !opening_cond
            raise(
              "no matching opening condition for closing condition "+
              "#{ident.to_loc_and_val_s}"
            )
          end
          if ident.value.strip != opening_cond.value.strip
            raise(
              "opening condition #{opening_cond.to_loc_and_val_s} "+
              " was closed with #{ident.to_loc_and_val_s}"
            )
          end
        end

      when :close then
        raise("unexpected closing token at #{tok.to_loc_and_val_s}")
      when :source then
        # source is fine to see outside of echo/cond/inv_cond/end_cond/comment
      else
        raise "unexpected token #{tok}"
      end
    end

    if cond_stack.any?
      c = cond_stack.shift
      raise(
        "no matching closing delimiter for #{c.to_loc_and_val_s}, got EOF"
      )
    end
  end
end

parser = Parser.new(toks)
parser.validate!
puts "parsed and validated mustache file."

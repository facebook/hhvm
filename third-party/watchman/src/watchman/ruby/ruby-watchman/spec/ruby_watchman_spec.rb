# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

require 'spec_helper'

describe RubyWatchman do
  def binary(str)
    if str.respond_to?(:force_encoding) # Ruby >= 1.9
      str.force_encoding('ASCII-8BIT')
    else
      str
    end
  end

  def little_endian?
    byte = [0xff00].pack('s')[0]
    if byte.is_a?(Fixnum) # ie. Ruby 1.8
      byte.zero?
    elsif byte.is_a?(String) # ie. Ruby >= 1.9
      byte == "\x00"
    else
      raise 'unable to determine endianness'
    end
  end

  def roundtrip(value)
    described_class.load(described_class.dump(value))
  end

  it 'roundtrips arrays' do
    value = [1, 2, ['three', false]]
    expect(roundtrip(value)).to eq(value)
  end

  it 'roundtrips hashes' do
    value = {
      'foo' => 1,
      'bar' => {
        'baz' => 'bing',
      }
    }
    expect(roundtrip(value)).to eq(value)
  end

  it 'roundtrips strings' do
    expect(roundtrip('')).to eq('')
    expect(roundtrip('/foo/bar/baz')).to eq('/foo/bar/baz')
  end

  it 'roundtrips uint8_t integers' do
    expect(roundtrip(0)).to eq(0)
    expect(roundtrip(1)).to eq(1)
    expect(roundtrip(0xff)).to eq(0xff)
  end

  it 'roundtrips uint16_t integers' do
    expect(roundtrip(0x1234)).to eq(0x1234)
  end

  it 'roundtrips uint32_t integers' do
    expect(roundtrip(0x12345678)).to eq(0x12345678)
  end

  it 'roundtrips uint64_t integers' do
    expect(roundtrip(0x12345678abcdef00)).to eq(0x12345678abcdef00)
  end

  it 'roundtrips floats' do
    expect(roundtrip(1234.5678)).to eq(1234.5678)
  end

  it 'roundtrips `true` booleans' do
    expect(roundtrip(true)).to eq(true)
  end

  it 'roundtrips `false` booleans' do
    expect(roundtrip(false)).to eq(false)
  end

  it 'roundtrips nil' do
    expect(roundtrip(nil)).to be_nil
  end

  describe '.load' do
    it 'rejects undersized input' do
      expect { described_class.load('') }.
        to raise_error(ArgumentError, /undersized/i)
    end

    it 'rejects input without a binary marker' do
      expect { described_class.load('gibberish') }.
        to raise_error(ArgumentError, /missing/i)
    end

    it 'rejects a missing payload header' do
      # binary protocol marker, but nothing else
      input = binary("\x00\x01")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /undersized/i)
    end

    it 'rejects empty payloads' do
      # uint8_t size marker of zero
      input = binary("\x00\x01\x03\x00")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /empty/i)
    end

    it 'rejects unrecognized payload markers' do
      # 0x10 is not a valid integer marker
      input = binary("\x00\x01\x10\x00")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /bad integer/i)
    end

    it 'rejects undersized payload markers' do
      # int16_t marker, but only storage for int8_t
      input = binary("\x00\x01\x04\x00")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /overrun\b.+\bint16_t/i)
    end

    it 'loads array values' do
      input = binary(
        "\x00\x01\x03\x16\x00\x03\x05\x03\x01\x02\x03" \
        "\x06foobar\x08\x09\x00\x03\x02\x03\x0a\x0a"
      )
      expect(described_class.load(input)).
        to eq([1, 'foobar', true, false, [10, nil]])
    end

    it 'handles empty arrays' do
      input = binary("\x00\x01\x03\x03\x00\x03\x00")
      expect(described_class.load(input)).to eq([])
    end

    it 'rejects arrays with incomplete headers' do
      input = binary("\x00\x01\x03\x02\x00\x03")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /incomplete array header/i)
    end

    it 'rejects arrays with incomplete entries' do
      input = binary("\x00\x01\x03\x05\x00\x03\x10\x0a\x0a")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/i)
    end

    it 'loads hash values' do
      input = binary(
        "\x00\x01\x03\x1a\x01\x03\x02\x02\x03\x03foo\x0a" \
        "\x02\x03\x03bar\x01\x03\x01\x02\x03\x03baz\x08"
      )
      expected = {
        'foo' => nil,
        'bar' => {
          'baz' => true,
        }
      }
      expect(described_class.load(input)).to eq(expected)
    end

    it 'handles empty hashes' do
      input = binary("\x00\x01\x03\x03\x01\x03\x00")
      expect(described_class.load(input)).to eq({})
    end

    it 'rejects hashes with incomplete headers' do
      input = binary("\x00\x01\x03\x02\x01\x03")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /incomplete hash header/i)
    end

    it 'rejects hashes with invalid keys' do
      # keys must be strings; this one uses uses a number instead
      input = binary("\x00\x01\x03\x05\x01\x03\x01\x03\x00")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /not a number/i)
    end

    it 'rejects hashes with missing keys' do
      input = binary("\x00\x01\x03\x03\x01\x03\x01")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/i)
    end

    it 'rejects hashes with missing values' do
      input = binary("\x00\x01\x03\x09\x01\x03\x01\x02\x03\x03foo")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/i)
    end

    it 'loads string values' do
      input = binary("\x00\x01\x03\x06\x02\x03\x03foo")
      expect(described_class.load(input)).to eq('foo')
    end

    it 'handles empty strings' do
      input = binary("\x00\x01\x03\x03\x02\x03\x00")
      expect(described_class.load(input)).to eq('')
    end

    if String.new.respond_to?(:encoding) # ie. Ruby >= 1.9
      it 'loads string values as ASCII-8BIT encoded strings' do
        input = binary("\x00\x01\x03\x06\x02\x03\x03foo")
        expect(described_class.load(input).encoding.to_s).to eq('ASCII-8BIT')
      end
    end

    it 'rejects string values with incomplete headers' do
      input = binary("\x00\x01\x03\x01\x02")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /invalid string header/i)
    end

    it 'rejects string values with invalid headers' do
      # expect a number indicating the string length, get a boolean instead
      input = binary("\x00\x01\x03\x05\x02\x08foo")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /bad integer/i)
    end

    it 'rejects string values with insufficient storage' do
      # expect 3 bytes, get 2 instead
      input = binary("\x00\x01\x03\x05\x02\x03\x03fo")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /insufficient string storage/i)
    end

    it 'loads uint8_t values' do
      input = binary("\x00\x01\x03\x02\x03\x12")
      expect(described_class.load(input)).to eq(0x12)
    end

    it 'loads uint16_t values' do
      if little_endian?
        input = binary("\x00\x01\x03\x03\x04\x34\x12")
      else
        input = binary("\x00\x01\x03\x03\x04\x12\x34")
      end

      expect(described_class.load(input)).to eq(0x1234)
    end

    it 'loads uint32_t values' do
      if little_endian?
        input = binary("\x00\x01\x03\x05\x05\x78\x56\x34\x12")
      else
        input = binary("\x00\x01\x03\x05\x05\x12\x34\x56\x78")
      end

      expect(described_class.load(input)).to eq(0x12345678)
    end

    it 'loads int uint64_t values' do
      if little_endian?
        input = binary("\x00\x01\x03\x09\x06\xef\xcd\xab\x90\x78\x56\x34\x12")
      else
        input = binary("\x00\x01\x03\x09\x06\x12\x34\x56\x78\x90\xab\xcd\xef")
      end
      expect(described_class.load(input)).to eq(0x1234567890abcdef)
    end

    it 'rejects int markers with missing values' do
      # expect an integer, but hit the end of the buffer
      input = binary("\x00\x01\x03\x01\x05")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /insufficient int storage/i)
    end

    it 'rejects double markers with insufficient storage' do
      # double with 7 bytes of storage instead of the required 8 bytes
      input = binary("\x00\x01\x03\x08\x07\x00\x00\x00\x00\x00\x00\x00")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /insufficient double storage/i)
    end

    it 'loads boolean `true` values' do
      input = binary("\x00\x01\x03\x01\x08")
      expect(described_class.load(input)).to eq(true)
    end

    it 'loads boolean `false` values' do
      input = binary("\x00\x01\x03\x01\x09")
      expect(described_class.load(input)).to eq(false)
    end

    it 'loads nil' do
      input = binary("\x00\x01\x03\x01\x0a")
      expect(described_class.load(input)).to be_nil
    end

    it 'loads templates' do
      # this example includes a "skip" marker
      input = binary(
        "\x00\x01\x03\x28\x0b\x00\x03\x02\x02\x03\x04name" \
        "\x02\x03\x03age\x03\x03\x02\x03\x04fred\x03" \
        "\x14\x02\x03\x04pete\x03\x1e\x0c\x03\x19"
      )
      expected = [
        { 'name' => 'fred', 'age' => 20 },
        { 'name' => 'pete', 'age' => 30 },
        { 'age' => 25  },
      ]
      expect(described_class.load(input)).to eq(expected)
    end

    it 'handles empty templates' do
      input = binary(
        "\x00\x01\x03\x12\x0b\x00\x03\x02\x02" \
        "\x03\x03foo\x02\x03\x03bar\x03\x00"
      )
      expect(described_class.load(input)).to eq([])
    end

    it 'rejects templates without a header array' do
      input = binary("\x00\x01\x03\x01\x0b")
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/i)
    end

    it 'rejects templates without a row items array' do
      input = binary(
        "\x00\x01\x03\x10\x0b\x00\x03\x02\x02" \
        "\x03\x03foo\x02\x03\x03bar"
      )
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /insufficient/i)
    end

    it 'rejects templates with non-string header items' do
      input = binary(
        "\x00\x01\x03\x0e\x0b\x00\x03\x02\x02" \
        "\x03\x03foo\x03\x03\x03\x00"
      )
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /not a number/)
    end

    it 'rejects templates with a header item array count mismatch' do
      input = binary(
        "\x00\x01\x03\x0a\x0b\x00\x03\x02\x02" \
        "\x03\x03foo"
      )
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/)
    end

    it 'rejects templates with a row item count mismatch' do
      input = binary(
        "\x00\x01\x03\x25\x0b\x00\x03\x02\x02\x03\x04name" \
        "\x02\x03\x03age\x03\x03\x02\x03\x04fred\x03" \
        "\x14\x02\x03\x04pete\x03\x1e"
      )
      expect { described_class.load(input) }.
        to raise_error(ArgumentError, /unexpected end/)
    end
  end

  describe '.dump' do
    let(:query) do
      # typical kind of query that Command-T issues
      ['query', '/some/path', {
        'expression' => ['type', 'f'],
        'fields'     => ['name'],
      }]
    end

    it 'serializes' do
      expect { described_class.dump(query) }.to_not raise_error
    end

    if String.new.respond_to?(:encoding) # ie. Ruby >= 1.9
      it 'serializes to an ASCII-8BIT string' do
        expect(described_class.dump(query).encoding.to_s).to eq('ASCII-8BIT')
      end
    end

    it 'generates a correct serialization' do
      # in Ruby 1.8, hashes aren't ordered, so two serializations are possible
      if little_endian?
        expected = [
          binary(
            "\x00\x01\x06\x49\x00\x00\x00\x00\x00\x00\x00\x00\x03\x03\x02\x03" \
            "\x05query\x02\x03\x0a/some/path\x01\x03\x02\x02\x03\x0a" \
            "expression\x00\x03\x02\x02\x03\x04type\x02\x03\x01f\x02\x03\x06" \
            "fields\x00\x03\x01\x02\x03\x04name"
          ),
          binary(
            "\x00\x01\x06\x49\x00\x00\x00\x00\x00\x00\x00\x00\x03\x03\x02\x03" \
            "\x05query\x02\x03\x0a/some/path\x01\x03\x02\x02\x03\x06fields" \
            "\x00\x03\x01\x02\x03\x04name\x02\x03\x0aexpression\x00\x03\x02" \
            "\x02\x03\x04type\x02\x03\x01f"
          )
        ]
      else
        expected = [
          binary(
            "\x00\x01\x06\x00\x00\x00\x00\x00\x00\x00\x49\x00\x03\x03\x02\x03" \
            "\x05query\x02\x03\x0a/some/path\x01\x03\x02\x02\x03\x0a" \
            "expression\x00\x03\x02\x02\x03\x04type\x02\x03\x01f\x02\x03\x06" \
            "fields\x00\x03\x01\x02\x03\x04name"
          ),
          binary(
            "\x00\x01\x06\x00\x00\x00\x00\x00\x00\x00\x49\x00\x03\x03\x02\x03" \
            "\x05query\x02\x03\x0a/some/path\x01\x03\x02\x02\x03\x06fields" \
            "\x00\x03\x01\x02\x03\x04name\x02\x03\x0aexpression\x00\x03\x02" \
            "\x02\x03\x04type\x02\x03\x01f"
          )
        ]
      end
      expect(expected).to include(described_class.dump(query))
    end
  end
end

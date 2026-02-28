/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.facebook.thrift;

import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.Assert.assertThat;

import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TCompactProtocol;
import com.facebook.thrift.protocol.TField;
import com.facebook.thrift.protocol.TMessage;
import com.facebook.thrift.protocol.TMessageType;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.protocol.TStruct;
import com.facebook.thrift.protocol.TType;
import com.facebook.thrift.server.TRpcConnectionContext;
import com.facebook.thrift.transport.TMemoryBuffer;
import java.util.Arrays;
import java.util.List;
import org.junit.Test;
import thrift.test.proto.CompactProtoTestStruct;
import thrift.test.proto.HolyMoley;
import thrift.test.proto.Nesting;
import thrift.test.proto.OneOfEach;
import thrift.test.proto.Srv;

public class TCompactProtocolTest {

  static TProtocolFactory factory = new TCompactProtocol.Factory();

  @Test
  public void testCompactProtocol() throws Exception {
    testNakedByte();
    for (int i = 0; i < 128; i++) {
      testByteField((byte) i);
      testByteField((byte) -i);
    }

    testNakedI16((short) 0);
    testNakedI16((short) 1);
    testNakedI16((short) 15000);
    testNakedI16((short) 0x7fff);
    testNakedI16((short) -1);
    testNakedI16((short) -15000);
    testNakedI16((short) -0x7fff);

    testI16Field((short) 0);
    testI16Field((short) 1);
    testI16Field((short) 7);
    testI16Field((short) 150);
    testI16Field((short) 15000);
    testI16Field((short) 0x7fff);
    testI16Field((short) -1);
    testI16Field((short) -7);
    testI16Field((short) -150);
    testI16Field((short) -15000);
    testI16Field((short) -0x7fff);

    testNakedI32(0);
    testNakedI32(1);
    testNakedI32(15000);
    testNakedI32(0xffff);
    testNakedI32(-1);
    testNakedI32(-15000);
    testNakedI32(-0xffff);

    testI32Field(0);
    testI32Field(1);
    testI32Field(7);
    testI32Field(150);
    testI32Field(15000);
    testI32Field(31337);
    testI32Field(0xffff);
    testI32Field(0xffffff);
    testI32Field(-1);
    testI32Field(-7);
    testI32Field(-150);
    testI32Field(-15000);
    testI32Field(-0xffff);
    testI32Field(-0xffffff);

    testNakedI64(0);
    for (int i = 0; i < 62; i++) {
      testNakedI64(1L << i);
      testNakedI64(-(1L << i));
    }

    testI64Field(0);
    for (int i = 0; i < 62; i++) {
      testI64Field(1L << i);
      testI64Field(-(1L << i));
    }

    testDouble();

    testFloat();

    testNakedString("");
    testNakedString("short");
    testNakedString("borderlinetiny");
    testNakedString("a bit longer than the smallest possible");

    testStringField("");
    testStringField("short");
    testStringField("borderlinetiny");
    testStringField("a bit longer than the smallest possible");

    testFloatField(123.456f);
    testFloatField(-1.0f);
    testFloatField(0.0f);
    testFloatField(Float.NaN);
    testFloatField(Float.POSITIVE_INFINITY);
    testFloatField(Float.NEGATIVE_INFINITY);

    testNakedBinary(new byte[] {});
    testNakedBinary(new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    testNakedBinary(new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14});
    testNakedBinary(new byte[128]);

    testBinaryField(new byte[] {});
    testBinaryField(new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    testBinaryField(new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14});
    testBinaryField(new byte[128]);

    testSerialization(OneOfEach.class, Fixtures.oneOfEach);
    testSerialization(Nesting.class, Fixtures.nesting);
    testSerialization(HolyMoley.class, Fixtures.holyMoley);
    testSerialization(CompactProtoTestStruct.class, Fixtures.compactProtoTestStruct);

    testMessage();

    testServerRequest();
  }

  @Test
  public void testNakedByte() throws Exception {
    byte x = 123;
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeByte(x);
    byte out = proto.readByte();
    assertThat(out, equalTo(x));
  }

  private static void testByteField(byte b) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.BYTE, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeByte(b);
          }

          public void readMethod(TProtocol proto) throws TException {
            byte result = proto.readByte();
            assertThat(result, equalTo(b));
          }
        });
  }

  private static void testNakedI16(short n) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeI16(n);
    short out = proto.readI16();
    assertThat(out, equalTo(n));
  }

  private static void testI16Field(short n) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.I16, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeI16(n);
          }

          public void readMethod(TProtocol proto) throws TException {
            short result = proto.readI16();
            assertThat(result, equalTo(n));
          }
        });
  }

  private static void testNakedI32(int n) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeI32(n);
    int out = proto.readI32();
    assertThat(out, equalTo(n));
  }

  private static void testI32Field(int n) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.I32, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeI32(n);
          }

          public void readMethod(TProtocol proto) throws TException {
            int result = proto.readI32();
            assertThat(result, equalTo(n));
          }
        });
  }

  private static void testNakedI64(long n) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeI64(n);
    long out = proto.readI64();
    assertThat(out, equalTo(n));
  }

  private static void testI64Field(long n) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.I64, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeI64(n);
          }

          public void readMethod(TProtocol proto) throws TException {
            long result = proto.readI64();
            assertThat(result, equalTo(n));
          }
        });
  }

  @Test
  public void testDouble() throws Exception {
    double d = 123.456;
    TMemoryBuffer buf = new TMemoryBuffer(1000);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeDouble(d);
    double out = proto.readDouble();
    assertThat(out, equalTo(d));
  }

  @Test
  public void testFloat() throws Exception {
    float f = 321.654f;
    TMemoryBuffer buf = new TMemoryBuffer(1000);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeFloat(f);
    float out = proto.readFloat();
    assertThat(out, equalTo(f));
  }

  private static void testFloatField(float value) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.FLOAT, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeFloat(value);
          }

          public void readMethod(TProtocol proto) throws TException {
            float result = proto.readFloat();
            assertThat(Float.compare(result, value), equalTo(0));
          }
        });
  }

  private static void testNakedString(String str) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeString(str);
    String out = proto.readString();
    assertThat(out, equalTo(str));
  }

  private static void testStringField(String str) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.STRING, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeString(str);
          }

          public void readMethod(TProtocol proto) throws TException {
            String result = proto.readString();
            assertThat(result, equalTo(str));
          }
        });
  }

  private static void testNakedBinary(byte[] data) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);
    proto.writeBinary(data);
    byte[] out = proto.readBinary();

    assertThat(out, is(data));
  }

  private static void testBinaryField(byte[] data) throws Exception {
    testStructField(
        new StructFieldTestCase(TType.STRING, (short) 15) {
          public void writeMethod(TProtocol proto) throws TException {
            proto.writeBinary(data);
          }

          public void readMethod(TProtocol proto) throws TException {
            byte[] result = proto.readBinary();
            assertThat(result, is(data));
          }
        });
  }

  private static <T extends TBase> void testSerialization(Class<T> klass, T obj) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TBinaryProtocol binproto = new TBinaryProtocol(buf);
    obj.write(binproto);

    buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);

    obj.write(proto);

    T objRead = klass.newInstance();
    objRead.read(proto);
    assertThat(obj, equalTo(objRead));
  }

  @Test
  public void testMessage() throws Exception {
    List<TMessage> msgs =
        Arrays.asList(
            new TMessage[] {
              new TMessage("short message name", TMessageType.CALL, 0),
              new TMessage("1", TMessageType.REPLY, 12345),
              new TMessage(
                  "loooooooooooooooooooooooooooooooooong", TMessageType.EXCEPTION, 1 << 16),
              new TMessage("Janky", TMessageType.CALL, 0),
              new TMessage("1way", TMessageType.ONEWAY, 54321),
            });

    for (TMessage msg : msgs) {
      TMemoryBuffer buf = new TMemoryBuffer(0);
      TProtocol proto = factory.getProtocol(buf);
      TMessage output = null;
      proto.writeMessageBegin(msg);
      proto.writeMessageEnd();
      output = proto.readMessageBegin();
      assertThat(output, equalTo(msg));
    }
  }

  @Test
  public void testServerRequest() throws Exception {
    Srv.Iface handler =
        new Srv.Iface() {
          public int Janky(int i32arg) throws TException {
            return i32arg * 2;
          }

          public int primitiveMethod() throws TException {
            // TODO Auto-generated method stub
            return 0;
          }

          public CompactProtoTestStruct structMethod() throws TException {
            // TODO Auto-generated method stub
            return null;
          }

          public void voidMethod() throws TException {
            // TODO Auto-generated method stub
          }
        };

    Srv.Processor testProcessor = new Srv.Processor(handler);

    TMemoryBuffer clientOutTrans = new TMemoryBuffer(0);
    TProtocol clientOutProto = factory.getProtocol(clientOutTrans);
    TMemoryBuffer clientInTrans = new TMemoryBuffer(0);
    TProtocol clientInProto = factory.getProtocol(clientInTrans);

    Srv.Client testClient = new Srv.Client(clientInProto, clientOutProto);

    testClient.send_Janky(1);
    TRpcConnectionContext ctx = new TRpcConnectionContext(null, clientInProto, clientOutProto);
    testProcessor.process(clientOutProto, clientInProto, ctx);
    int result = testClient.recv_Janky();
    assertThat(result, equalTo(2));
  }

  //
  // Helper methods
  //

  private static void testStructField(StructFieldTestCase testCase) throws Exception {
    TMemoryBuffer buf = new TMemoryBuffer(0);
    TProtocol proto = factory.getProtocol(buf);

    TField field = new TField("test_field", testCase.type_, testCase.id_);
    proto.writeStructBegin(new TStruct("test_struct"));
    proto.writeFieldBegin(field);
    testCase.writeMethod(proto);
    proto.writeFieldEnd();
    proto.writeStructEnd();

    proto.readStructBegin();
    TField readField = proto.readFieldBegin();

    assertThat(readField.id, equalTo(field.id));
    assertThat(readField.type, equalTo(field.type));
    // TODO: verify the field content is as expected
    testCase.readMethod(proto);
    proto.readStructEnd();
  }

  public abstract static class StructFieldTestCase {
    byte type_;
    short id_;

    public StructFieldTestCase(byte type, short id) {
      type_ = type;
      id_ = id;
    }

    public abstract void writeMethod(TProtocol proto) throws TException;

    public abstract void readMethod(TProtocol proto) throws TException;
  }
}

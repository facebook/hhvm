/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

package thrift.test.JsonToThriftTest;

import static org.junit.Assert.*;

import com.facebook.thrift.TBase;
import com.facebook.thrift.protocol.TCompactJSONProtocol;
import com.facebook.thrift.protocol.TJSONProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TIOStreamTransport;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import org.junit.Test;
import thrift.test.*;

/**
 * Test for compact JSON protocol, focusing on its match of output with simple JSON protocol, and
 * support for reading JSON back.
 */
public class TCompactJSONProtocolTest {
  private static TProtocolFactory compactProtocolFactory = new TCompactJSONProtocol.Factory();
  private static TProtocolFactory fullProtocolFactory = new TJSONProtocol.Factory();

  /** Write struct to JSON, using given protocol. */
  private static String write(TProtocolFactory protocolFactory, TBase s) throws Exception {
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    TIOStreamTransport transport = new TIOStreamTransport(null, outputStream);
    TProtocol protocol = protocolFactory.getProtocol(transport);
    s.write(protocol);
    String utf8Encoding = outputStream.toString("UTF-8");
    return utf8Encoding;
  }

  /** Read struct from JSON, using given protocol. */
  private static void read(TProtocolFactory protocolFactory, String string, TBase s)
      throws Exception {
    byte[] byteEncoding = string.getBytes("UTF-8");
    ByteArrayInputStream inputStream = new ByteArrayInputStream(byteEncoding);
    TIOStreamTransport transport = new TIOStreamTransport(inputStream, null);
    TProtocol protocol = protocolFactory.getProtocol(transport);
    s.read(protocol);
  }

  /** Used for debugging. */
  private static void debugOutput(TBase s) throws Exception {
    System.out.println(s.getClass());
    System.out.println("  TCompactJSONProtocol: " + write(compactProtocolFactory, s));
    System.out.println("  TJSONProtocol:        " + write(fullProtocolFactory, s));
    System.out.println();
  }

  /**
   * Assert that compact protocol can read its own output back, and that the result is the same. If
   * matchBytewise is set, then both input and output are written as JSON, and equality of it
   * asserted, too.
   */
  private static void assertReadBack(TBase s, boolean matchBytewise) throws Exception {
    String string = write(compactProtocolFactory, s);
    TBase dummy = s.getClass().newInstance();
    read(compactProtocolFactory, string, dummy);
    String dummyString = write(compactProtocolFactory, dummy);
    if (matchBytewise) {
      assertEquals(string, dummyString);
    }
    assertTrue(dummy.equals(s));
  }

  /** Annotate structs for debug output. */
  @Target(ElementType.FIELD)
  @Retention(RetentionPolicy.RUNTIME)
  public @interface DebugOutput {}

  /** Annotate structs for assertion of compact protocol reading JSON back. */
  @Target(ElementType.FIELD)
  @Retention(RetentionPolicy.RUNTIME)
  public @interface ReadBack {
    /** HashMap may result in different ordering, so bytewise comparison is generally skipped. */
    boolean matchBytewise() default false;
  }

  /** If any structs are annotated, only those are considered. */
  @Target(ElementType.FIELD)
  @Retention(RetentionPolicy.RUNTIME)
  public @interface Filter {}

  @ReadBack
  private static myBinaryStruct binaryStruct =
      new myBinaryStruct(new byte[] {'x', 'y', 'z', 'z', 'y'});

  @ReadBack private static myBoolStruct boolStruct1 = new myBoolStruct(true);

  @ReadBack private static myBoolStruct boolStruct2 = new myBoolStruct(false);

  @ReadBack private static myByteStruct byteStruct = new myByteStruct((byte) 101);

  // No bad case as Java's byte is equivalent to Thrift's byte
  // private static myByteStruct byteStructBad = new myByteStruct(3232);

  @ReadBack
  private static myComplexStruct complexStruct1 =
      new myComplexStruct(
          new mySimpleStruct(
              true, (byte) 92, (short) 902, 65536, 123456789, 3.1415, "Whan that Aprille"),
          Arrays.asList((short) 314, (short) 15, (short) 9, (short) 26535),
          new HashMap<String, mySimpleStruct>() {
            {
              put(
                  "qwerty",
                  new mySimpleStruct() {
                    {
                      setC((short) 1);
                    }
                  });
              put(
                  "slippy",
                  new mySimpleStruct() {
                    {
                      setA(false);
                      setB((byte) -4);
                      setC((short) 5);
                    }
                  });
            }
          },
          EnumTest.EnumTwo,
          new ExceptionTest("test"));

  // Skipped because Java expects valid enum value for myComplexStruct.b
  // private static myComplexStruct complexStruct2 = new myComplexStruct();

  @ReadBack private static myDoubleStruct doubleStruct1 = new myDoubleStruct(-2.192);

  @ReadBack
  private static myDoubleStruct doubleStruct2 = new myDoubleStruct(Double.POSITIVE_INFINITY);

  @ReadBack
  private static myDoubleStruct doubleStruct3 = new myDoubleStruct(Double.NEGATIVE_INFINITY);

  @ReadBack private static myI16Struct I16Struct = new myI16Struct((short) 4567);

  // No bad case as Java's short is equivalent to Thrift's I16
  // private static myI16Struct I16StructBad = new myI16Struct(0xFEDCBA987);

  @ReadBack private static myI32Struct I32Struct = new myI32Struct(12131415);

  // No bad case as Java's int is equivalent to Thrift's I32
  // private static myI32Struct I32StructBad = new myI32Struct(0xFFFFFFFFEDCBA);

  @ReadBack
  private static myMixedStruct mixedStruct =
      new myMixedStruct(
          new ArrayList<Short>(),
          Arrays.asList(new mySuperSimpleStruct((short) 5)),
          new HashMap<String, Short>() {
            {
              put("flame", (short) -8);
              put("fire", (short) -191);
            }
          },
          new HashMap<String, mySuperSimpleStruct>(),
          new HashSet<Short>(Arrays.asList((short) 1, (short) 2, (short) 3, (short) 4)));

  @ReadBack
  private static mySetStruct setStruct1 =
      new mySetStruct(
          new HashSet<Short>(Arrays.asList((short) 4, (short) 8, (short) 15, (short) 16)));

  @ReadBack private static mySetStruct setStruct2 = new mySetStruct(new HashSet<Short>());

  // No bad case as Java's short is equivalent to Thrift's I16
  // private static mySetStruct setStructBad = new mySetStruct(set([1, 0xFFFFFFFFFF, 2]));

  @ReadBack
  private static myMapStruct mapStruct =
      new myMapStruct(
          new HashMap<String, String>() {
            {
              put("a", "A");
              put("b", "B");
            }
          },
          new HashMap<Boolean, String>() {
            {
              put(true, "True");
              put(false, "False");
            }
          },
          new HashMap<Byte, String>() {
            {
              put((byte) 1, "one");
              put((byte) 2, "two");
            }
          },
          new HashMap<Double, String>() {
            {
              put(0.1, "0.one");
              put(0.2, "0.two");
            }
          },
          new HashMap<Gender, String>() {
            {
              put(Gender.MALE, "male");
              put(Gender.FEMALE, "female");
            }
          });

  @ReadBack
  private static myNestedMapStruct nestedMapStruct =
      new myNestedMapStruct(
          new HashMap<String, Map<String, mySimpleStruct>>() {
            {
              put(
                  "1",
                  new HashMap<String, mySimpleStruct>() {
                    {
                      put(
                          "1",
                          new mySimpleStruct() {
                            {
                              setC((short) 1);
                            }
                          });
                    }
                  });
              put(
                  "2",
                  new HashMap<String, mySimpleStruct>() {
                    {
                      put(
                          "2",
                          new mySimpleStruct() {
                            {
                              setA(false);
                              setC((short) 2);
                            }
                          });
                    }
                  });
            }
          });

  @ReadBack
  private static mySimpleStruct simpleStruct1 =
      new mySimpleStruct(false, (byte) 87, (short) 7880, -7880, (long) -1, -0.1, "T-bone");

  @ReadBack
  private static mySimpleStruct simpleStruct2 =
      new mySimpleStruct() {
        {
          setC((short) 9);
        }
      };

  @ReadBack private static mySimpleStruct simpleStructBad = new mySimpleStruct();

  @ReadBack private static myStringStruct stringStruct1 = new myStringStruct("");

  @ReadBack private static myStringStruct stringStruct2 = new myStringStruct();

  @ReadBack private static myStringStruct stringStruct3 = new myStringStruct("foobar");

  @Test
  public void testStructs() throws Exception {
    // See if any structs are annotated for filtering
    boolean filterOnly = false;
    for (Field field : getClass().getDeclaredFields()) {
      if (field.isAnnotationPresent(Filter.class)) {
        filterOnly = true;
        break;
      }
    }

    for (Field field : getClass().getDeclaredFields()) {
      if (filterOnly) {
        // Only consider filtered structs
        if (!field.isAnnotationPresent(Filter.class)) {
          continue;
        }
      }

      if (field.isAnnotationPresent(DebugOutput.class)) {
        debugOutput((TBase) field.get(null));
      }

      ReadBack readBackAnnotation = (ReadBack) field.getAnnotation(ReadBack.class);

      if (readBackAnnotation != null) {
        assertReadBack((TBase) field.get(null), readBackAnnotation.matchBytewise());
      }
    }
  }

  @Test
  public void testUnknown() throws Exception {
    String original = write(compactProtocolFactory, complexStruct1);

    String modified =
        "{\"_k\":13,\"_l\":[],\"_m\":{},\"_n\":[[{}],[{\"_o\":[]}]],"
            + original.substring(1, original.length() - 1)
            + ",\"_x\":-5,\"_y\":0.1,\"_z\":1}";

    myComplexStruct actual = new myComplexStruct();

    read(compactProtocolFactory, modified, actual);

    assertTrue(actual.equals(complexStruct1));
  }

  private static class BoolTestCase {
    public String inputJson;
    public boolean value;
    public String outputJson;

    public BoolTestCase(String inputJson, boolean value, String outputJson) {
      this.inputJson = inputJson;
      this.value = value;
      this.outputJson = outputJson;
    }

    public BoolTestCase(String inputJson, boolean value) {
      this(inputJson, value, inputJson);
    }
  }

  private static BoolTestCase boolTestCases[] =
      new BoolTestCase[] {
        new BoolTestCase("{\"a\":1}", true, "{\"a\":1}"),
        new BoolTestCase("{\"a\":0}", false, "{\"a\":0}"),
        new BoolTestCase("{\"a\":true}", true, "{\"a\":1}"),
        new BoolTestCase("{\"a\":false}", false, "{\"a\":0}"),
        new BoolTestCase("{\"a\":123}", true, "{\"a\":1}")
      };

  @Test
  public void testBool() throws Exception {
    for (BoolTestCase testCase : boolTestCases) {
      myBoolStruct instance = new myBoolStruct();
      read(compactProtocolFactory, testCase.inputJson, instance);
      assertTrue(testCase.inputJson, instance.isSetA());
      assertEquals(testCase.inputJson, testCase.value, instance.isA());
      String actualOutputJson = write(compactProtocolFactory, instance);
      assertEquals(testCase.inputJson, testCase.outputJson, actualOutputJson);
    }
  }
}

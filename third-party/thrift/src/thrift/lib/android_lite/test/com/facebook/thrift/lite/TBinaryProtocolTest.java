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

package com.facebook.thrift.lite;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import com.facebook.thrift.lite.protocol.TBinaryProtocol;
import com.facebook.thrift.lite.protocol.TField;
import com.facebook.thrift.lite.protocol.TList;
import com.facebook.thrift.lite.protocol.TMap;
import com.facebook.thrift.lite.protocol.TSet;
import com.facebook.thrift.lite.protocol.TType;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

public class TBinaryProtocolTest {

  private static final String mOutputFileName = "IgByteArrayTest.tmp";
  private File tempFile;
  private TBinaryProtocol binaryProtocol;
  private FileOutputStream outputFile;
  private FileInputStream inputFile;
  private byte[] mInputBuffer;

  @Rule public TemporaryFolder folder = new TemporaryFolder();

  @Before
  public void setupProtocol() throws IOException {
    tempFile = folder.newFile(mOutputFileName);
    outputFile = new FileOutputStream(tempFile, false);
    BufferedOutputStream byteArray = new BufferedOutputStream(outputFile);
    binaryProtocol = new TBinaryProtocol(byteArray);
  }

  @After
  public void closeAndDeleteFile() throws IOException {
    inputFile.close();
    assertTrue(tempFile.delete());
  }

  @Test
  public void testWriteIntegers() throws Exception {
    binaryProtocol.writeByte((byte) 0xfb);
    binaryProtocol.writeI16((short) 0xface);
    binaryProtocol.writeI32(0xfaceb00c);
    binaryProtocol.writeI64(0xfaceb00cfaceb00cL);

    closeOutputFileAndOpenInputFile();

    int offset = 0;
    // tests byte
    assertEquals(mInputBuffer[0], (byte) 0xfb);
    offset++;

    // tests short (i16)
    compareShortToBinary((short) 0xface, mInputBuffer, offset);
    offset += 2;

    // tests int (i32)
    compareIntToBinary(0xfaceb00c, mInputBuffer, offset);
    offset += 4;

    // tests long (i64)
    compareLongToBinary(0xfaceb00cfaceb00cL, mInputBuffer, offset);
    offset += 8;
    assertEquals(mInputBuffer.length, offset);
  }

  @Test
  public void testWriteReals() throws Exception {
    binaryProtocol.writeFloat(-123.456f);
    binaryProtocol.writeDouble(123456.7890);

    closeOutputFileAndOpenInputFile();

    compareIntToBinary(Float.floatToIntBits(-123.456f), mInputBuffer, 0);
    compareLongToBinary(Double.doubleToLongBits(123456.7890), mInputBuffer, 4);
  }

  @Test
  public void testWriteCollections() throws Exception {
    binaryProtocol.writeMapBegin(new TMap(TType.DOUBLE, TType.STRUCT, 1234));
    binaryProtocol.writeListBegin(new TList(TType.ENUM, 256));
    binaryProtocol.writeSetBegin(new TSet(TType.MAP, 0xffffffff));
    binaryProtocol.writeFieldBegin(new TField("This is just a field name", TType.I64, (short) 17));

    closeOutputFileAndOpenInputFile();

    int offset = 0;

    // tests map
    assertEquals(TType.DOUBLE, mInputBuffer[offset]);
    offset++;
    assertEquals(TType.STRUCT, mInputBuffer[offset]);
    offset++;
    compareIntToBinary(1234, mInputBuffer, offset);
    offset += 4;

    // tests list
    assertEquals(TType.ENUM, mInputBuffer[offset]);
    offset++;
    compareIntToBinary(256, mInputBuffer, offset);
    offset += 4;

    // tests set
    assertEquals(TType.MAP, mInputBuffer[offset]);
    offset++;
    compareIntToBinary(0xffffffff, mInputBuffer, offset);
    offset += 4;

    // tests field
    assertEquals(TType.I64, mInputBuffer[offset]);
    offset++;
    compareShortToBinary((short) 17, mInputBuffer, offset);
  }

  @Test
  public void testWriteString() throws Exception {
    String testString = "This is just a test string with a bunch of characters. #yolo";
    binaryProtocol.writeString(testString);

    closeOutputFileAndOpenInputFile();

    compareBufferToBinary(testString.getBytes("UTF-8"), mInputBuffer, 0);
  }

  @Test
  public void testWriteStringOverflow() throws Exception {
    String baseString = "This is part of a long string that will be pasted repeatedly. #yolo";
    StringBuffer buffer = new StringBuffer();
    int numRepeats = 100;
    for (int i = 0; i < numRepeats; i++) {
      buffer.append(baseString);
    }
    String testString = buffer.toString();
    binaryProtocol.writeString(testString);

    closeOutputFileAndOpenInputFile();

    compareBufferToBinary(testString.getBytes("UTF-8"), mInputBuffer, 0);
  }

  @Test
  public void testWriteBinary() throws Exception {
    int size = 9876;
    byte[] testBinary = new byte[size];
    for (int i = 0; i < size; i++) {
      testBinary[i] = (byte) (Math.random() * 100);
    }
    binaryProtocol.writeBinary(testBinary);

    closeOutputFileAndOpenInputFile();

    compareBufferToBinary(testBinary, mInputBuffer, 0);
  }

  private void closeOutputFileAndOpenInputFile() throws IOException {
    // first flushes the stream and closes the file
    binaryProtocol.getTransport().flush();
    outputFile.close();
    // now open the file for reading and points the input buffer to it
    inputFile = new FileInputStream(tempFile);
    mInputBuffer = copyFileToByteArray(inputFile);
  }

  private static byte[] copyFileToByteArray(InputStream inputStream) throws IOException {
    ByteArrayOutputStream buffer = new ByteArrayOutputStream();

    int nRead;
    byte[] data = new byte[8 * 1024];

    while ((nRead = inputStream.read(data, 0, data.length)) != -1) {
      buffer.write(data, 0, nRead);
    }

    buffer.flush();
    return buffer.toByteArray();
  }

  // thrift strings/binaries are serialized as length (4 bytes) + raw bytes
  private static void compareBufferToBinary(byte[] firstArray, byte[] secondArray, int offset) {
    // adds 4 as the size of the string is serialized as an int in the binary
    assertTrue(secondArray.length >= firstArray.length + offset + 4);

    // checks that the length of the string in the binary buffer matches
    compareIntToBinary(firstArray.length, secondArray, 0);

    for (int i = 0; i < firstArray.length; i++) {
      assertEquals(firstArray[i], secondArray[i + 4]);
    }
  }

  private static void compareShortToBinary(short input, byte[] byteArray, int offset) {
    assertTrue(byteArray.length >= offset + 2);
    short output = 0;
    for (int i = 0; i < 2; i++) {
      output <<= 8;
      output += (0xff & byteArray[offset + i]);
    }
    assertEquals(input, output);
  }

  private static void compareIntToBinary(int input, byte[] byteArray, int offset) {
    assertTrue(byteArray.length >= offset + 4);
    int output = 0;
    for (int i = 0; i < 4; i++) {
      output <<= 8;
      output += (0xff & byteArray[offset + i]);
    }
    assertEquals(input, output);
  }

  private static void compareLongToBinary(long input, byte[] byteArray, int offset) {
    assertTrue(byteArray.length >= offset + 8);
    long output = 0;
    for (int i = 0; i < 8; i++) {
      output <<= 8;
      output += (0xff & byteArray[offset + i]);
    }
    assertEquals(input, output);
  }
}

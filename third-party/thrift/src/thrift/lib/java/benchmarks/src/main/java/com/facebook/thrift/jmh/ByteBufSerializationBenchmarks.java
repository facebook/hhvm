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

package com.facebook.thrift.jmh;

import com.facebook.thrift.protocol.ByteBufTProtocol;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.test.EveryLayout;
import com.facebook.thrift.test.Nested1;
import com.facebook.thrift.test.Nested2;
import com.facebook.thrift.test.Pet;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.TearDown;
import org.openjdk.jmh.infra.Blackhole;

public class ByteBufSerializationBenchmarks {

  @State(Scope.Benchmark)
  public static class Input {
    @Param({"TBinary", "TCompact"})
    public String protocol;

    Blackhole bh;
    TProtocolType protocolType;
    ByteBufAllocator alloc;

    Pet fluffy;
    EveryLayout everyLayout;
    Nested2 nested;

    ByteBuf fluffyBytes;
    ByteBuf everyLayoutBytes;
    ByteBuf nestedBytes;

    ByteBuf target;

    @Setup
    public void setup(Blackhole bh) throws Exception {
      this.alloc = ByteBufAllocator.DEFAULT;
      this.target = alloc.buffer();
      this.protocolType = getTProtocol();
      this.bh = bh;

      setupSmall();
      setupBig();
      setupNested();
    }

    @TearDown
    public void teardown() {
      fluffyBytes.release();
      everyLayoutBytes.release();
      nestedBytes.release();
      target.release();
    }

    private TProtocolType getTProtocol() {
      if ("TBinary".equals(protocol)) {
        return TProtocolType.TBinary;
      } else {
        return TProtocolType.TCompact;
      }
    }

    private void setupSmall() throws Exception {
      fluffy = new Pet.Builder().setName("Fluffy").setAge(100_000).setVegan(true).build();

      fluffyBytes = alloc.buffer();
      ByteBufTProtocol byteBufTProtocol = protocolType.apply(fluffyBytes);
      fluffy.write0(byteBufTProtocol);
    }

    private void setupBig() throws Exception {
      List<String> l = new ArrayList<>();
      Set<String> s = new HashSet<>();
      Map<Integer, String> m = new HashMap<>();
      for (int i = 0; i < 10; i++) {
        String e = String.valueOf(i);
        l.add(e);
        s.add(e);
        m.put(i, e);
      }

      everyLayout =
          new EveryLayout.Builder()
              .setABool(false)
              .setAInt(42)
              .setAList(l)
              .setAFloat(1.0F)
              .setASet(s)
              .setAMap(m)
              .build();

      everyLayoutBytes = alloc.buffer();
      ByteBufTProtocol byteBufTProtocol = protocolType.apply(everyLayoutBytes);
      everyLayout.write0(byteBufTProtocol);
    }

    private void setupNested() throws Exception {
      Map<Integer, Nested1> nests = new HashMap<>();
      for (int j = 0; j < 10; j++) {
        List<Pet> pets = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
          Pet p = new Pet.Builder().setName("Fluffy").setAge(1).setVegan(true).build();
          pets.add(p);
        }
        Nested1 nested1 = new Nested1.Builder().setPets(pets).build();
        nests.put(j, nested1);
      }

      nested = new Nested2.Builder().setNests(nests).build();

      nestedBytes = alloc.buffer();
      ByteBufTProtocol byteBufTProtocol = protocolType.apply(nestedBytes);
      nested.write0(byteBufTProtocol);
    }
  }

  @Benchmark
  public void benchmarkSmallSerialize(Input input) throws Exception {
    ByteBuf target = input.target.clear();
    ByteBufTProtocol protocol = input.protocolType.apply(target);
    protocol.wrap(target);
    input.fluffy.write0(protocol);
    input.bh.consume(target);
  }

  @Benchmark
  public void benchmarkSmallDeserialize(Input input) throws Exception {
    input.fluffyBytes.markReaderIndex();
    ByteBufTProtocol protocol = input.protocolType.apply(input.fluffyBytes);
    protocol.wrap(input.fluffyBytes);
    Pet read = Pet.read0(protocol);
    input.fluffyBytes.resetReaderIndex();
    input.bh.consume(read);
  }

  @Benchmark
  public void benchmarkBigSerialize(Input input) throws Exception {
    ByteBuf target = input.target.clear();
    ByteBufTProtocol protocol = input.protocolType.apply(target);
    protocol.wrap(target);
    input.everyLayout.write0(protocol);
    input.bh.consume(target);
  }

  @Benchmark
  public void benchmarkBigDeserialize(Input input) throws Exception {
    input.everyLayoutBytes.markReaderIndex();
    ByteBufTProtocol protocol = input.protocolType.apply(input.everyLayoutBytes);
    protocol.wrap(input.everyLayoutBytes);
    EveryLayout read = EveryLayout.read0(protocol);
    input.everyLayoutBytes.resetReaderIndex();
    input.bh.consume(read);
  }

  @Benchmark
  public void benchmarkNestedSerialize(Input input) throws Exception {
    ByteBuf target = input.target.clear();
    ByteBufTProtocol protocol = input.protocolType.apply(target);
    protocol.wrap(target);
    input.nested.write0(protocol);
    input.bh.consume(target);
  }

  @Benchmark
  public void benchmarkNestedDeserialize(Input input) throws Exception {
    input.nestedBytes.markReaderIndex();
    ByteBufTProtocol protocol = input.protocolType.apply(input.nestedBytes);
    protocol.wrap(input.nestedBytes);
    Nested2 read = Nested2.read0(protocol);
    input.bh.consume(read);
    input.nestedBytes.resetReaderIndex();
  }
}

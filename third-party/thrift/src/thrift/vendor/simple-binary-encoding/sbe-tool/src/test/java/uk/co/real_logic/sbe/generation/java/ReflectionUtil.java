/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package uk.co.real_logic.sbe.generation.java;

import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import uk.co.real_logic.sbe.generation.Generators;

final class ReflectionUtil
{
    static int getSbeSchemaVersion(final Object encoder) throws Exception
    {
        return getInt(encoder, "sbeSchemaVersion");
    }

    static int getSbeBlockLength(final Object encoder) throws Exception
    {
        return getInt(encoder, "sbeBlockLength");
    }

    static int getInt(final Object object, final String fieldName) throws Exception
    {
        return (int)get(object, fieldName);
    }

    static Object get(final Object object, final String fieldName) throws Exception
    {
        return object.getClass().getMethod(fieldName).invoke(object);
    }

    static Object get(final Object object, final String fieldName, final Appendable arg) throws Exception
    {
        final String methodName = "get" + Generators.toUpperFirstChar(fieldName);
        return object.getClass().getMethod(methodName, Appendable.class).invoke(object, arg);
    }

    static void skipFuelFiguresGroup(final Object decoder) throws Exception
    {
        skipGroup(decoder, "fuelFigures");
    }

    static void skipPerformanceFiguresGroup(final Object decoder) throws Exception
    {
        skipGroup(decoder, "performanceFigures");
    }

    private static void skipGroup(final Object decoder, final String groupName) throws Exception
    {
        final Object group = get(decoder, groupName);
        while ((boolean)get(group, "hasNext"))
        {
            get(group, "next");
            get(group, "sbeSkip");
        }
    }

    static String getManufacturer(final Object decoder) throws Exception
    {
        return (String)get(decoder, "manufacturer");
    }

    static void setEmptyFuelFiguresGroup(final Object encoder) throws Exception
    {
        encoder.getClass().getMethod("fuelFiguresCount", int.class).invoke(encoder, 0);
    }

    static void setEmptyPerformanceFiguresGroup(final Object encoder) throws Exception
    {
        encoder.getClass().getMethod("performanceFiguresCount", int.class).invoke(encoder, 0);
    }

    static void setManufacturer(final Object encoder, final String value) throws Exception
    {
        encoder.getClass().getMethod("manufacturer", String.class).invoke(encoder, value);
    }

    static void putSerialNumber(final Object encoder, final long serial) throws Exception
    {
        encoder
            .getClass()
            .getMethod("serialNumber", long.class)
            .invoke(encoder, serial);
    }

    static long getSerialNumber(final Object reader) throws Exception
    {
        return (long)reader.getClass().getMethod("serialNumber").invoke(reader);
    }

    static int getLimit(final Object flyweight) throws Exception
    {
        return getInt(flyweight, "limit");
    }

    static Object getExtras(final Object object) throws Exception
    {
        return get(object, "extras");
    }

    static Object setCruiseControl(final Object extras, final boolean value) throws Exception
    {
        return extras.getClass().getMethod("cruiseControl", boolean.class).invoke(extras, value);
    }

    static boolean getCruiseControl(final Object extras) throws Exception
    {
        return (boolean)extras.getClass().getMethod("cruiseControl").invoke(extras);
    }

    static int getCapacity(final Object flyweight) throws Exception
    {
        return getInt(flyweight, "capacity");
    }

    static void setCapacity(final Object flyweight, final int value) throws Exception
    {
        flyweight
            .getClass()
            .getMethod("capacity", int.class)
            .invoke(flyweight, value);
    }

    static void set(
        final Object object,
        final String name,
        final Class<?> type,
        final Object value) throws Exception
    {
        object.getClass().getMethod(name, type).invoke(object, value);
    }

    static void putByteArray(
        final Object encoder,
        final String name,
        final byte[] value,
        final int offset,
        final int length) throws Exception
    {
        encoder.getClass().getDeclaredMethod(name, byte[].class, int.class, int.class)
            .invoke(encoder, value, offset, length);
    }

    static int getByteArray(
        final Object decoder,
        final String name,
        final byte[] dst,
        final int dstOffset,
        final int length) throws Exception
    {
        return (Integer)decoder.getClass().getDeclaredMethod(name, byte[].class, int.class, int.class)
            .invoke(decoder, dst, dstOffset, length);
    }

    static void putDirectBuffer(
        final Object encoder,
        final String name,
        final DirectBuffer value,
        final int offset,
        final int length) throws Exception
    {
        encoder.getClass().getDeclaredMethod(name, DirectBuffer.class, int.class, int.class)
                .invoke(encoder, value, offset, length);
    }

    static int getDirectBuffer(
        final Object decoder,
        final String name,
        final MutableDirectBuffer dst,
        final int dstOffset,
        final int length) throws Exception
    {
        return (Integer)decoder.getClass().getDeclaredMethod(name, MutableDirectBuffer.class, int.class, int.class)
            .invoke(decoder, dst, dstOffset, length);
    }

    static void wrapDirectBuffer(final Object decoder, final String name, final DirectBuffer dst) throws Exception
    {
        decoder
            .getClass()
            .getDeclaredMethod(name, DirectBuffer.class)
            .invoke(decoder, dst);
    }

    static Object getByte(final Class<?> clazz, final byte value) throws Exception
    {
        return clazz.getDeclaredMethod("get", byte.class).invoke(null, value);
    }

    static Object getFuelFigures(final Object msgFlyweight) throws Exception
    {
        return get(msgFlyweight, "fuelFigures");
    }

    static Object fuelFiguresCount(final Object msgFlyweight, final int count) throws Exception
    {
        return msgFlyweight.getClass().getMethod("fuelFiguresCount", int.class).invoke(msgFlyweight, count);
    }

    static int getCount(final Object groupFlyweight) throws Exception
    {
        return getInt(groupFlyweight, "count");
    }
}

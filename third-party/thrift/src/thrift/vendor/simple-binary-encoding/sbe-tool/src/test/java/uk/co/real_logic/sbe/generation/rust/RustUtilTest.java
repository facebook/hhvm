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
package uk.co.real_logic.sbe.generation.rust;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static uk.co.real_logic.sbe.PrimitiveType.CHAR;
import static uk.co.real_logic.sbe.PrimitiveType.DOUBLE;
import static uk.co.real_logic.sbe.PrimitiveType.FLOAT;
import static uk.co.real_logic.sbe.PrimitiveType.INT16;
import static uk.co.real_logic.sbe.PrimitiveType.INT32;
import static uk.co.real_logic.sbe.PrimitiveType.INT64;
import static uk.co.real_logic.sbe.PrimitiveType.INT8;
import static uk.co.real_logic.sbe.PrimitiveType.UINT16;
import static uk.co.real_logic.sbe.PrimitiveType.UINT32;
import static uk.co.real_logic.sbe.PrimitiveType.UINT64;
import static uk.co.real_logic.sbe.PrimitiveType.UINT8;
import static uk.co.real_logic.sbe.generation.rust.RustUtil.cleanUpperAcronyms;
import static uk.co.real_logic.sbe.generation.rust.RustUtil.formatFunctionName;
import static uk.co.real_logic.sbe.generation.rust.RustUtil.generateRustLiteral;

class RustUtilTest
{
    @Test
    void nullParamToEightBitCharacterThrowsNPE()
    {
        assertThrows(NullPointerException.class, () -> RustUtil.eightBitCharacter(null));
    }

    @Test
    void emptyParamToEightBitCharacterThrowsIAE()
    {
        assertThrows(IllegalArgumentException.class, () -> RustUtil.eightBitCharacter(""));
    }

    @Test
    void tooManyCharactersParamToEightBitCharacterThrowsIAE()
    {
        assertThrows(IllegalArgumentException.class, () -> RustUtil.eightBitCharacter("ABC"));
    }

    @Test
    void happyPathEightBitCharacter()
    {
        final byte aByte = RustUtil.eightBitCharacter("a");
        assertEquals('a', (char)aByte);
        assertEquals("97", Byte.toString(aByte));
    }

    @Test
    void generateRustLiteralsHappyPaths()
    {
        assertEquals("65_u8", generateRustLiteral(CHAR, "65"));
        assertEquals("64.1_f64", generateRustLiteral(DOUBLE, "64.1"));
        assertEquals("f64::NAN", generateRustLiteral(DOUBLE, "NaN"));
        assertEquals("64.1_f32", generateRustLiteral(FLOAT, "64.1"));
        assertEquals("f32::NAN", generateRustLiteral(FLOAT, "NaN"));

        assertEquals("65_i8", generateRustLiteral(INT8, "65"));
        assertEquals("65_i16", generateRustLiteral(INT16, "65"));
        assertEquals("65_i32", generateRustLiteral(INT32, "65"));
        assertEquals("65_i64", generateRustLiteral(INT64, "65"));

        assertEquals("0x41_u8", generateRustLiteral(UINT8, "65"));
        assertEquals("0x41_u16", generateRustLiteral(UINT16, "65"));
        assertEquals("0x41_u32", generateRustLiteral(UINT32, "65"));
        assertEquals("0x41_u64", generateRustLiteral(UINT64, "65"));
    }

    @Test
    void generateRustLiteralsForUnsignedPrimitiveNulls()
    {
        assertEquals("0xff_u8", generateRustLiteral(UINT8, UINT8.nullValue().toString()));
        assertEquals("0xffff_u16", generateRustLiteral(UINT16, UINT16.nullValue().toString()));
        assertEquals("0xffffffff_u32", generateRustLiteral(UINT32, UINT32.nullValue().toString()));
        assertEquals("0xffffffffffffffff_u64", generateRustLiteral(UINT64, UINT64.nullValue().toString()));
    }

    @Test
    void generateRustLiteralNullPrimitiveTypeParam()
    {
        assertThrows(NullPointerException.class, () -> generateRustLiteral(null, "65"));
    }

    @Test
    void generateRustLiteralNullValueParam()
    {
        assertThrows(NullPointerException.class, () -> generateRustLiteral(INT8, null));
    }

    @Test
    void testCleanUpperAcronyms()
    {
        assertEquals("ABee", cleanUpperAcronyms("ABee"));
        assertEquals("mdEntryTypeBook", cleanUpperAcronyms("MDEntryTypeBook"));
        assertEquals("MD_EntryTypeBook", cleanUpperAcronyms("MD_EntryTypeBook"));
        assertEquals("price9Book", cleanUpperAcronyms("PRICE9Book"));
    }

    @Test
    void functionNameCasing()
    {
        assertEquals("", formatFunctionName(""));
        assertEquals("a", formatFunctionName("a"));
        assertEquals("a", formatFunctionName("A"));
        assertEquals("car", formatFunctionName("Car"));
        assertEquals("car", formatFunctionName("car"));
        assertEquals("decode_car", formatFunctionName("DecodeCar"));
        assertEquals("decode_car", formatFunctionName("decodeCar"));
        assertEquals("decode_car", formatFunctionName("decode_car"));
        assertEquals("decode_car", formatFunctionName("Decode_car"));
        assertEquals("decode_car", formatFunctionName("decode_Car"));
        assertEquals("decode_car", formatFunctionName("Decode_Car"));
        assertEquals("decode_car", formatFunctionName("DECODE_Car"));
        assertEquals("decode_car", formatFunctionName("DECODE_car"));
        assertEquals("decode_car", formatFunctionName("DECODECar"));
        assertEquals("decode_car", formatFunctionName("DECODE_CAR"));
        assertEquals("decode_ca_r", formatFunctionName("DECODE_caR"));

        // special cases
        assertEquals("pricenull_9", formatFunctionName("PRICENULL9"));
        assertEquals("price_9_book", formatFunctionName("PRICE9Book"));
        assertEquals("issue_435", formatFunctionName("issue435"));
        assertEquals("r#type", formatFunctionName("type"));

        assertEquals("upper_case", formatFunctionName("UPPERCase"));
        assertEquals("no_md_entries", formatFunctionName("NoMDEntries"));
        assertEquals("md_entry_type_book", formatFunctionName("MD_EntryTYPEBook"));
        assertEquals("cl_ord_id", formatFunctionName("ClOrdID"));
        assertEquals("ab_c", formatFunctionName("aBc"));
        assertEquals("ab_cd", formatFunctionName("aBcD"));
        assertEquals("ab_cd", formatFunctionName("aB_cD"));
        assertEquals("ab_cd", formatFunctionName("AbCd"));
    }
}

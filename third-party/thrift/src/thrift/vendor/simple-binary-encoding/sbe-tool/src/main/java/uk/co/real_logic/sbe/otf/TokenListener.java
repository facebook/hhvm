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
package uk.co.real_logic.sbe.otf;

import org.agrona.DirectBuffer;
import uk.co.real_logic.sbe.ir.Token;

import java.util.List;

/**
 * Callback interface to be implemented by code wanting to decode messages on-the-fly.
 * <p>
 * If all methods are not required then consider extending {@link AbstractTokenListener} for simpler code.
 */
public interface TokenListener
{
    /**
     * Called on beginning the decoding of a message.
     *
     * @param token representing the IR for message including metadata.
     */
    void onBeginMessage(Token token);

    /**
     * Called on end of decoding of a message.
     *
     * @param token representing the IR for message including metadata.
     */
    void onEndMessage(Token token);

    /**
     * Primitive encoded type encountered. This can be a root block field or field within a composite or group.
     * <p>
     * Within a composite the typeToken and fieldToken are the same.
     *
     * @param fieldToken    in the IR representing the field of the message root or group.
     * @param buffer        containing the encoded message.
     * @param bufferIndex   at which the encoded field begins.
     * @param typeToken     of the encoded primitive value.
     * @param actingVersion of the encoded message for determining validity of extension fields.
     */
    void onEncoding(Token fieldToken, DirectBuffer buffer, int bufferIndex, Token typeToken, int actingVersion);

    /**
     * Enum encoded type encountered.
     *
     * @param fieldToken    in the IR representing the field of the message root or group.
     * @param buffer        containing the encoded message.
     * @param bufferIndex   at which the encoded field begins.
     * @param tokens        describing the message.
     * @param fromIndex     at which the enum metadata begins.
     * @param toIndex       at which the enum metadata ends.
     * @param actingVersion of the encoded message for determining validity of extension fields.
     */
    void onEnum(
        Token fieldToken,
        DirectBuffer buffer,
        int bufferIndex,
        List<Token> tokens,
        int fromIndex,
        int toIndex,
        int actingVersion);

    /**
     * BitSet encoded type encountered.
     *
     * @param fieldToken    in the IR representing the field of the message root or group.
     * @param buffer        containing the encoded message.
     * @param bufferIndex   at which the encoded field begins.
     * @param tokens        describing the message.
     * @param fromIndex     at which the bit set metadata begins.
     * @param toIndex       at which the bit set metadata ends.
     * @param actingVersion of the encoded message for determining validity of extension fields.
     */
    void onBitSet(
        Token fieldToken,
        DirectBuffer buffer,
        int bufferIndex,
        List<Token> tokens,
        int fromIndex,
        int toIndex,
        int actingVersion);

    /**
     * Beginning of Composite encoded type encountered.
     *
     * @param fieldToken in the IR representing the field of the message root or group.
     * @param tokens     describing the message.
     * @param fromIndex  at which the composite metadata begins.
     * @param toIndex    at which the composite metadata ends.
     */
    void onBeginComposite(Token fieldToken, List<Token> tokens, int fromIndex, int toIndex);

    /**
     * End of Composite encoded type encountered.
     *
     * @param fieldToken in the IR representing the field of the message root or group.
     * @param tokens     describing the message.
     * @param fromIndex  at which the composite metadata begins.
     * @param toIndex    at which the composite metadata ends.
     */
    void onEndComposite(Token fieldToken, List<Token> tokens, int fromIndex, int toIndex);

    /**
     * Group encountered.
     *
     * @param token      describing the group.
     * @param numInGroup number of times the group will be repeated.
     */
    void onGroupHeader(Token token, int numInGroup);

    /**
     * Beginning of group encoded type encountered.
     *
     * @param token      describing the group.
     * @param groupIndex index for the repeat count of the group.
     * @param numInGroup number of times the group will be repeated.
     */
    void onBeginGroup(Token token, int groupIndex, int numInGroup);

    /**
     * End of group encoded type encountered.
     *
     * @param token      describing the group.
     * @param groupIndex index for the repeat count of the group.
     * @param numInGroup number of times the group will be repeated.
     */
    void onEndGroup(Token token, int groupIndex, int numInGroup);

    /**
     * Var data field encountered.
     *
     * @param fieldToken  in the IR representing the var data field.
     * @param buffer      containing the encoded message.
     * @param bufferIndex at which the variable data begins.
     * @param length      of the variable data in bytes.
     * @param typeToken   of the variable data. Needed to determine character encoding of the variable data.
     */
    void onVarData(Token fieldToken, DirectBuffer buffer, int bufferIndex, int length, Token typeToken);
}

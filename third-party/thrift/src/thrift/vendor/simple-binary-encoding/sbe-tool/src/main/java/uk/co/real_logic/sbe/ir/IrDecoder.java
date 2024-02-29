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
package uk.co.real_logic.sbe.ir;

import org.agrona.CloseHelper;
import org.agrona.MutableDirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.ir.generated.FrameCodecDecoder;
import uk.co.real_logic.sbe.ir.generated.TokenCodecDecoder;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

import static java.nio.file.StandardOpenOption.READ;
import static uk.co.real_logic.sbe.ir.IrUtil.*;

/**
 * Decoder for encoded {@link Ir} representing an SBE schema which can be read from a buffer or file.
 */
public class IrDecoder implements AutoCloseable
{
    private static final int CAPACITY = 4096;

    private final FileChannel channel;
    private final MutableDirectBuffer directBuffer;
    private final FrameCodecDecoder frameDecoder = new FrameCodecDecoder();
    private final TokenCodecDecoder tokenDecoder = new TokenCodecDecoder();
    private int offset;
    private final int length;
    private String irPackageName = null;
    private String irNamespaceName = null;
    private String semanticVersion = null;
    private List<Token> irHeader = null;
    private int irId;
    private int irVersion = 0;
    private final byte[] valArray = new byte[CAPACITY];
    private final MutableDirectBuffer valBuffer = new UnsafeBuffer(valArray);

    /**
     * Construct a {@link Ir} decoder by opening a file for a given name.
     *
     * @param fileName containing the encoded {@link Ir}.
     */
    public IrDecoder(final String fileName)
    {
        try
        {
            channel = FileChannel.open(Paths.get(fileName), READ);
            final long fileLength = channel.size();

            if (fileLength > Integer.MAX_VALUE)
            {
                throw new IllegalStateException("Invalid IR file: length=" + fileLength + " > Integer.MAX_VALUE");
            }

            final MappedByteBuffer buffer = channel.map(FileChannel.MapMode.READ_ONLY, 0, fileLength);
            directBuffer = new UnsafeBuffer(buffer);
            length = (int)fileLength;
            offset = 0;
        }
        catch (final IOException ex)
        {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Construct a {@link Ir} decoder for data encoded in a {@link ByteBuffer}.
     *
     * @param buffer containing the serialised {@link Ir}.
     */
    public IrDecoder(final ByteBuffer buffer)
    {
        channel = null;
        length = buffer.limit();
        directBuffer = new UnsafeBuffer(buffer);
        offset = 0;
    }

    /**
     * {@inheritDoc}
     */
    public void close()
    {
        CloseHelper.quietClose(channel);
    }

    /**
     * Decode the serialised {@link Ir} and return the decoded instance.
     *
     * @return the decoded serialised {@link Ir} instance.
     */
    public Ir decode()
    {
        decodeFrame();

        final List<Token> tokens = new ArrayList<>();
        while (offset < length)
        {
            tokens.add(decodeToken());
        }

        int i = 0;
        if (tokens.get(0).signal() == Signal.BEGIN_COMPOSITE)
        {
            i = captureHeader(tokens);
        }

        final ByteOrder byteOrder = !tokens.isEmpty() ? tokens.get(0).encoding().byteOrder() : null;
        final Ir ir = new Ir(
            irPackageName, irNamespaceName, irId, irVersion, null, semanticVersion, byteOrder, irHeader);

        for (int size = tokens.size(); i < size; i++)
        {
            if (tokens.get(i).signal() == Signal.BEGIN_MESSAGE)
            {
                i = captureMessage(tokens, i, ir);
            }
        }

        return ir;
    }

    private int captureHeader(final List<Token> tokens)
    {
        final List<Token> headerTokens = new ArrayList<>();

        int index = 0;
        Token token = tokens.get(index);
        final String headerName = token.name();
        headerTokens.add(token);
        do
        {
            token = tokens.get(++index);
            headerTokens.add(token);
        }
        while (Signal.END_COMPOSITE != token.signal() || !headerName.equals(token.name()));

        irHeader = headerTokens;

        return index;
    }

    private static int captureMessage(final List<Token> tokens, final int index, final Ir ir)
    {
        final List<Token> messageTokens = new ArrayList<>();

        int i = index;
        Token token = tokens.get(i);
        messageTokens.add(token);
        do
        {
            token = tokens.get(++i);
            messageTokens.add(token);
        }
        while (Signal.END_MESSAGE != token.signal());

        ir.addMessage(tokens.get(i).id(), messageTokens);

        return i;
    }

    private void decodeFrame()
    {
        frameDecoder.wrap(directBuffer, offset, frameDecoder.sbeBlockLength(), FrameCodecDecoder.SCHEMA_VERSION);

        irId = frameDecoder.irId();

        if (frameDecoder.irVersion() != 0)
        {
            throw new IllegalStateException("Unknown SBE version: " + frameDecoder.irVersion());
        }

        irVersion = frameDecoder.schemaVersion();
        irPackageName = frameDecoder.packageName();

        irNamespaceName = frameDecoder.namespaceName();
        if (irNamespaceName.isEmpty())
        {
            irNamespaceName = null;
        }

        semanticVersion = frameDecoder.semanticVersion();
        if (semanticVersion.isEmpty())
        {
            semanticVersion = null;
        }

        offset += frameDecoder.encodedLength();
    }

    private Token decodeToken()
    {
        final Token.Builder tokenBuilder = new Token.Builder();
        final Encoding.Builder encBuilder = new Encoding.Builder();

        tokenDecoder.wrap(directBuffer, offset, tokenDecoder.sbeBlockLength(), TokenCodecDecoder.SCHEMA_VERSION);

        tokenBuilder
            .offset(tokenDecoder.tokenOffset())
            .size(tokenDecoder.tokenSize())
            .id(tokenDecoder.fieldId())
            .version(tokenDecoder.tokenVersion())
            .componentTokenCount(tokenDecoder.componentTokenCount())
            .signal(mapSignal(tokenDecoder.signal()));

        final PrimitiveType type = mapPrimitiveType(tokenDecoder.primitiveType());

        encBuilder
            .primitiveType(mapPrimitiveType(tokenDecoder.primitiveType()))
            .byteOrder(mapByteOrder(tokenDecoder.byteOrder()))
            .presence(mapPresence(tokenDecoder.presence()));

        tokenBuilder.name(tokenDecoder.name());

        encBuilder.constValue(get(valBuffer, type, tokenDecoder.getConstValue(valArray, 0, valArray.length)));
        encBuilder.minValue(get(valBuffer, type, tokenDecoder.getMinValue(valArray, 0, valArray.length)));
        encBuilder.maxValue(get(valBuffer, type, tokenDecoder.getMaxValue(valArray, 0, valArray.length)));
        encBuilder.nullValue(get(valBuffer, type, tokenDecoder.getNullValue(valArray, 0, valArray.length)));

        final String characterEncoding = tokenDecoder.characterEncoding();
        encBuilder.characterEncoding(characterEncoding.isEmpty() ? null : characterEncoding);

        final String epoch = tokenDecoder.epoch();
        encBuilder.epoch(epoch.isEmpty() ? null : epoch);

        final String timeUnit = tokenDecoder.timeUnit();
        encBuilder.timeUnit(timeUnit.isEmpty() ? null : timeUnit);

        final String semanticType = tokenDecoder.semanticType();
        encBuilder.semanticType(semanticType.isEmpty() ? null : semanticType);

        final String description = tokenDecoder.description();
        tokenBuilder.description(description.isEmpty() ? null : description);

        final String referencedName = tokenDecoder.referencedName();
        tokenBuilder.referencedName(referencedName.isEmpty() ? null : referencedName);

        offset += tokenDecoder.encodedLength();

        return tokenBuilder.encoding(encBuilder.build()).build();
    }
}

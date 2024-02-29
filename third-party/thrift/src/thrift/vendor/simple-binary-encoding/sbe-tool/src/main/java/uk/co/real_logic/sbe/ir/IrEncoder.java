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
import org.agrona.LangUtil;
import org.agrona.MutableDirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.ir.generated.FrameCodecEncoder;
import uk.co.real_logic.sbe.ir.generated.TokenCodecEncoder;
import org.agrona.Verify;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Paths;
import java.util.List;

import static java.nio.file.StandardOpenOption.*;
import static uk.co.real_logic.sbe.ir.IrUtil.*;
import static uk.co.real_logic.sbe.ir.generated.FrameCodecEncoder.namespaceNameCharacterEncoding;
import static uk.co.real_logic.sbe.ir.generated.FrameCodecEncoder.packageNameCharacterEncoding;
import static uk.co.real_logic.sbe.ir.generated.FrameCodecEncoder.semanticVersionCharacterEncoding;
import static uk.co.real_logic.sbe.ir.generated.TokenCodecEncoder.*;

/**
 * Encoder for {@link Ir} representing an SBE schema which can be written to a buffer or file.
 */
public class IrEncoder implements AutoCloseable
{
    private static final int CAPACITY = 4096;

    private final FileChannel channel;
    private final ByteBuffer resultBuffer;
    private final ByteBuffer buffer;
    private final MutableDirectBuffer directBuffer;
    private final Ir ir;
    private final FrameCodecEncoder frameEncoder = new FrameCodecEncoder();
    private final TokenCodecEncoder tokenEncoder = new TokenCodecEncoder();
    private final byte[] valArray = new byte[CAPACITY];
    private final UnsafeBuffer valBuffer = new UnsafeBuffer(valArray);
    private int totalLength = 0;

    /**
     * Construct an encoder for {@link Ir} to a file. An existing file will be overwritten.
     *
     * @param fileName into which the {@link Ir} will be encoded.
     * @param ir       to be encoded into the file.
     */
    public IrEncoder(final String fileName, final Ir ir)
    {
        try
        {
            channel = FileChannel.open(Paths.get(fileName), READ, WRITE, CREATE, TRUNCATE_EXISTING);
            resultBuffer = null;
            buffer = ByteBuffer.allocateDirect(CAPACITY);
            directBuffer = new UnsafeBuffer(buffer);
            this.ir = ir;
        }
        catch (final IOException ex)
        {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Construct an encoder for {@link Ir} to a {@link ByteBuffer}.
     *
     * @param buffer into which the {@link Ir} will be encoded.
     * @param ir     to be encoded into the buffer.
     */
    public IrEncoder(final ByteBuffer buffer, final Ir ir)
    {
        channel = null;
        resultBuffer = buffer;
        this.buffer = ByteBuffer.allocateDirect(CAPACITY);
        directBuffer = new UnsafeBuffer(this.buffer);
        this.ir = ir;
    }

    /**
     * {@inheritDoc}
     */
    public void close()
    {
        CloseHelper.quietClose(channel);
    }

    /**
     * Encode the provided {@link Ir} and return the length in bytes encoded.
     *
     * @return encode the provided {@link Ir} and return the length in bytes encoded.
     */
    public int encode()
    {
        Verify.notNull(ir, "ir");

        write(buffer, encodeFrame());

        encodeTokenList(ir.headerStructure().tokens());

        ir.messages().forEach(this::encodeTokenList);

        return totalLength;
    }

    private void encodeTokenList(final List<Token> tokenList)
    {
        for (final Token token : tokenList)
        {
            write(buffer, encodeToken(token));
        }
    }

    private void write(final ByteBuffer buffer, final int length)
    {
        buffer.position(0);
        buffer.limit(length);

        if (channel != null)
        {
            try
            {
                channel.write(buffer);
            }
            catch (final IOException ex)
            {
                LangUtil.rethrowUnchecked(ex);
            }
        }
        else if (resultBuffer != null)
        {
            resultBuffer.put(buffer);
        }

        totalLength += length;
    }

    private int encodeFrame()
    {
        frameEncoder
            .wrap(directBuffer, 0)
            .irId(ir.id())
            .irVersion(0)
            .schemaVersion(ir.version());

        try
        {
            final byte[] packageBytes = ir.packageName().getBytes(packageNameCharacterEncoding());
            frameEncoder.putPackageName(packageBytes, 0, packageBytes.length);

            final byte[] namespaceBytes = getBytes(ir.namespaceName(), namespaceNameCharacterEncoding());
            frameEncoder.putNamespaceName(namespaceBytes, 0, namespaceBytes.length);

            final byte[] semanticVersionBytes = getBytes(ir.semanticVersion(), semanticVersionCharacterEncoding());
            frameEncoder.putSemanticVersion(semanticVersionBytes, 0, semanticVersionBytes.length);
        }
        catch (final UnsupportedEncodingException ex)
        {
            LangUtil.rethrowUnchecked(ex);
        }

        return frameEncoder.encodedLength();
    }

    private int encodeToken(final Token token)
    {
        final Encoding encoding = token.encoding();
        final PrimitiveType type = encoding.primitiveType();

        tokenEncoder
            .wrap(directBuffer, 0)
            .tokenOffset(token.offset())
            .tokenSize(token.encodedLength())
            .fieldId(token.id())
            .tokenVersion(token.version())
            .componentTokenCount(token.componentTokenCount())
            .signal(mapSignal(token.signal()))
            .primitiveType(mapPrimitiveType(type))
            .byteOrder(mapByteOrder(encoding.byteOrder()))
            .presence(mapPresence(encoding.presence()));

        try
        {
            final byte[] nameBytes = token.name().getBytes(TokenCodecEncoder.nameCharacterEncoding());
            tokenEncoder.putName(nameBytes, 0, nameBytes.length);

            tokenEncoder.putConstValue(valArray, 0, put(valBuffer, encoding.constValue(), type));
            tokenEncoder.putMinValue(valArray, 0, put(valBuffer, encoding.minValue(), type));
            tokenEncoder.putMaxValue(valArray, 0, put(valBuffer, encoding.maxValue(), type));
            tokenEncoder.putNullValue(valArray, 0, put(valBuffer, encoding.nullValue(), type));

            final byte[] charEncodingBytes = getBytes(
                encoding.characterEncoding(), characterEncodingCharacterEncoding());
            tokenEncoder.putCharacterEncoding(charEncodingBytes, 0, charEncodingBytes.length);

            final byte[] epochBytes = getBytes(encoding.epoch(), epochCharacterEncoding());
            tokenEncoder.putEpoch(epochBytes, 0, epochBytes.length);

            final byte[] timeUnitBytes = getBytes(encoding.timeUnit(), timeUnitCharacterEncoding());
            tokenEncoder.putTimeUnit(timeUnitBytes, 0, timeUnitBytes.length);

            final byte[] semanticTypeBytes = getBytes(encoding.semanticType(), semanticTypeCharacterEncoding());
            tokenEncoder.putSemanticType(semanticTypeBytes, 0, semanticTypeBytes.length);

            final byte[] descriptionBytes = getBytes(token.description(), descriptionCharacterEncoding());
            tokenEncoder.putDescription(descriptionBytes, 0, descriptionBytes.length);

            final byte[] referencedNameBytes = getBytes(token.referencedName(), referencedNameCharacterEncoding());
            tokenEncoder.putReferencedName(referencedNameBytes, 0, referencedNameBytes.length);
        }
        catch (final UnsupportedEncodingException ex)
        {
            LangUtil.rethrowUnchecked(ex);
        }

        return tokenEncoder.encodedLength();
    }
}

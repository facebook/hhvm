/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Devious Baton Protocol Client Implementation
// Based on draft-frindell-webtrans-devious-baton-00

class DeviousBatonClient {
  constructor(serverUrl, options = {}) {
    // Default options
    this.options = {
      version: 0,
      baton: null, // Server will choose random if null
      count: 1,
      ...options
    };

    this.serverUrl = new URL(serverUrl);
    this.transport = null;
    this.activebatons = 0;
    this.closed = false;

    // Add query parameters
    this.serverUrl.searchParams.set('version', this.options.version);
    if (this.options.baton !== null) {
      if (this.options.baton < 1 || this.options.baton > 255) {
        throw new Error('Baton value must be between 1 and 255');
      }
      this.serverUrl.searchParams.set('baton', this.options.baton);
    }
    this.serverUrl.searchParams.set('count', this.options.count);

    // Event listeners
    this.onConnected = null;
    this.onBatonReceived = null;
    this.onBatonSent = null;
    this.onDatagramReceived = null;
    this.onDatagramSent = null;
    this.onComplete = null;
    this.onError = null;
  }

  async connect() {
    try {
      // Create WebTransport connection with deviousbaton-01 subprotocol
      this.transport = new WebTransport(this.serverUrl.toString(), {
        protocols: ["deviousbaton-01"]
      });

      // Wait for connection to be established
      await this.transport.ready;

      // Set the number of active batons
      this.activebatons = this.options.count;

      // Set up event listeners
      this.setupEventListeners();

      // Call the connected callback if provided
      if (this.onConnected) {
        this.onConnected();
      }

      return true;
    } catch (error) {
      if (this.onError) {
        this.onError(error);
      }
      return false;
    }
  }

  setupEventListeners() {
      this.setupUnidirectionalStreamListener();
      this.setupBidirectionalStreamListener();
      this.setupDatagramListener();
      this.setupConnectionClosedListener();
  }

  async setupUnidirectionalStreamListener() {
      try {
          const reader = this.transport.incomingUnidirectionalStreams.getReader();
          let done = false;
          while (!done) {
              const { done: streamDone, value } = await reader.read();
              done = streamDone;
              if (!done) {
                  // Process each incoming unidirectional stream
                  await this.handleIncomingStream(value, value, "unidirectional");
              }
          }
      } catch (error) {
          if (this.onError) {
              this.onError(error);
          }
      }
  }

  async setupBidirectionalStreamListener() {
      try {
          const reader = this.transport.incomingBidirectionalStreams.getReader();
          let done = false;
          while (!done) {
              const { done: streamDone, value } = await reader.read();
              done = streamDone;
              if (!done) {
                  // Process each incoming bidirectional stream
                  await this.handleIncomingStream(value, value.readable, "bidirectional");
              }
          }
      } catch (error) {
          if (this.onError) {
              this.onError(error);
          }
      }
  }

  async setupDatagramListener() {
      if (this.transport.datagrams) {
          try {
              const datagramReader = this.transport.datagrams.readable.getReader();
              let done = false;
              while (!done) {
                  const { done: datagramDone, value } = await datagramReader.read();
                  done = datagramDone;
                  if (!done) {
                      // Process the datagram
                      await this.handleDatagram(value);
                  }
              }
          } catch (error) {
              if (this.onError) {
                  this.onError(error);
              }
          }
      }
  }

  async setupConnectionClosedListener() {
      try {
          await this.transport.closed;
          this.closed = true;
          if (this.onComplete && this.activebatons === 0) {
              this.onComplete();
          }
      } catch (error) {
          this.closed = true;
          if (this.onError) {
              this.onError(error);
          }
      }
  }

  async handleIncomingStream(stream, readable, streamType) {
    try {
      // Read the baton message from the stream
      const batonMessage = await this.readBatonMessage(readable);

      // Notify about received baton
      if (this.onBatonReceived) {
        this.onBatonReceived(batonMessage.baton, streamType);
      }

      // Process the baton according to protocol
      await this.processBaton(batonMessage.baton, streamType, stream);
    } catch (error) {
      // Handle stream processing error
      if (this.onError) {
        this.onError(error);
      }

      // Try to reset the stream with I_LIED error code
      try {
        if (stream.writable && !stream.writable.locked) {
          await stream.resetWithErrorCode(0x03); // I_LIED
        }
      } catch (resetError) {
        // Ignore reset errors
      }
    }
  }

  async readBatonMessage(readable) {
    const reader = readable.getReader();
    let buffer = new Uint8Array(0); // Buffer to hold data read from the stream
    let paddingLength = 0;

    // Helper function to read a certain number of bytes from the stream,
    // taking into account the buffer
    async function readBytes(numBytes) {
      while (buffer.length < numBytes) {
        const result = await reader.read();
        if (result.done) {
          throw new Error("Stream ended unexpectedly");
        }
        // Append the new chunk to the buffer
        const newBuffer = new Uint8Array(buffer.length + result.value.length);
        newBuffer.set(buffer);
        newBuffer.set(result.value, buffer.length);
        buffer = newBuffer;
      }

      // Extract the requested bytes and update the buffer
      const bytes = buffer.slice(0, numBytes);
      buffer = buffer.slice(numBytes);
      return bytes;
    }

    try {
      // Read the padding length (QUIC variable-length integer)
      let currentByte;
      let shift = 0;
      let byteCount = 0;
      let value = 0;

      while (true) {
        currentByte = await readBytes(1); // Read one byte at a time for decoding
        const byte = currentByte[0];

        const len = (byte & 0xC0) >> 6;
        byteCount = 1 << len;

        value = (byte & (0xFF >> (len + 2)));

        if (byteCount > 1) {
          // For multi-byte varints, we need to read additional bytes
          for (let i = 1; i < byteCount; i++) {
            currentByte = await readBytes(1);
            value = (value << 8) | currentByte[0];
          }
        }

        break;
      }

      paddingLength = value;

      // Skip over the padding bytes
      await readBytes(paddingLength);

      // Read the baton value (1 byte)
      const baton = await readBytes(1);

      return { paddingLength, baton: baton[0] };
    } finally {
      // Release the reader's lock
      reader.releaseLock();
    }
  }

  async processBaton(batonValue, incomingStreamType, incomingStream) {
    // If baton is 0, decrement active batons count
    if (batonValue === 0) {
      this.activebatons--;
      if (this.activebatons === 0) {
        await this.close();
      }
      return;
    }

    // Calculate the new baton value
    const newBatonValue = (batonValue + 1) % 256;

    // Send datagram if baton value meets the condition
    if (batonValue % 7 === 1) {
      await this.sendDatagram(batonValue);
    }

    // Choose the appropriate stream type based on how the baton arrived
    let stream;
    let outgoingStream;
    let outgoingStreamType;

    if (incomingStreamType === "unidirectional") {
      // If baton arrived on unidirectional stream, open a bidirectional stream
      stream = await this.transport.createBidirectionalStream();
      outgoingStream = stream.writable;
      outgoingStreamType = "bidirectional-self";
      this.handleIncomingStream(stream, stream.readable, outgoingStreamType);
    } else if (incomingStreamType === "bidirectional") {
      // If baton arrived on peer-initiated bidirectional stream, use the same stream
      stream = incomingStream;
      outgoingStream = incomingStream.writable;
      outgoingStreamType = "bidirectional-peer";
    } else if (incomingStreamType === "bidirectional-self") {
      // If baton arrived on self-initiated bidirectional stream, open a unidirectional stream
      stream = outgoingStream = await this.transport.createUnidirectionalStream();
      outgoingStreamType = "unidirectional";
    }

    // Send the baton message
    await this.sendBatonMessage(stream, outgoingStream, newBatonValue, outgoingStreamType);
  }

  async sendBatonMessage(stream, writable, batonValue, streamType) {
    try {
      // Create the baton message
      // For simplicity, use small padding (we could make this configurable)
      const paddingLength = 2; // Small padding value
      const messageSize = 1 + paddingLength + 1; // paddingLength byte + padding + baton byte
      const message = new Uint8Array(messageSize);

      // Set padding length
      message[0] = paddingLength;

      // Padding bytes (can be anything, using zeros)
      for (let i = 0; i < paddingLength; i++) {
        message[1 + i] = 0;
      }

      // Baton value
      message[messageSize - 1] = batonValue;

      // Send the baton message
      const writer = writable.getWriter();
      await writer.write(message);
      await writer.close();

      // Notify about sent baton
      if (this.onBatonSent) {
        this.onBatonSent(batonValue, streamType);
      }
    } catch (error) {
      if (this.onError) {
        this.onError(error);
      }

      // Try to reset the stream with I_LIED error code
      try {
        await stream.resetWithErrorCode(0x03); // I_LIED
      } catch (resetError) {
        // Ignore reset errors
      }
    }
  }

  async sendDatagram(batonValue) {
    try {
      if (!this.transport.datagrams) {
        return;
      }

      // Create a small datagram with the baton message
      // Using minimal padding to ensure it fits in a datagram
      const paddingLength = 0;
      const messageSize = 1 + paddingLength + 1; // paddingLength byte + padding + baton byte
      const message = new Uint8Array(messageSize);

      // Set padding length
      message[0] = paddingLength;

      // Baton value
      message[messageSize - 1] = batonValue;

      // Send the datagram
      const writer = this.transport.datagrams.writable.getWriter();
      await writer.write(message);
      writer.releaseLock();

      // Notify about sent datagram
      if (this.onDatagramSent) {
        this.onDatagramSent(batonValue);
      }
    } catch (error) {
      if (this.onError) {
        this.onError(error);
      }
    }
  }

  async handleDatagram(data) {
    try {
      // Extract baton value from datagram
      // For simplicity, assuming minimal datagram format
      const paddingLength = data[0];
      const batonValue = data[data.length - 1];

      // Notify about received datagram
      if (this.onDatagramReceived) {
        this.onDatagramReceived(batonValue);
      }
    } catch (error) {
      if (this.onError) {
        this.onError(error);
      }
    }
  }

  async close() {
    if (!this.closed) {
      try {
        await this.transport.close();
        this.closed = true;

        // Call the complete callback if all batons are done
        if (this.onComplete && this.activebatons === 0) {
          this.onComplete();
        }
      } catch (error) {
        if (this.onError) {
          this.onError(error);
        }
      }
    }
  }
}

// Example usage for browser
if (typeof window !== 'undefined') {
  window.DeviousBatonClient = DeviousBatonClient;
}

// Export for Node.js
if (typeof module !== 'undefined') {
  module.exports = { DeviousBatonClient };
}

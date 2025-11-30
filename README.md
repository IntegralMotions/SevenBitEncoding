
# SevenBitEncoding

A compact 7‑bit packing and variable‑length integer encoding utility for C++.

## Overview

This module provides helpers for:
- Variable‑length integer (varint‑style) encoding/decoding.
- Packing arbitrary binary buffers into 7‑bit aligned streams.
- Unpacking 7‑bit encoded streams back to raw bytes.

Useful for serial protocols, embedded message framing, and bandwidth‑sensitive communication.

## Install using fetch content

``` CMAKE
include(FetchContent)

FetchContent_Declare(
    SevenBitEncoding
    GIT_REPOSITORY https://github.com/youruser/SevenBitEncoding.git
    GIT_TAG        v1.0.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SevenBitEncoding)

target_link_libraries(MyApp PRIVATE SevenBitEncoding::SevenBitEncoding)
```

## Features

### Integer Encoding
- `getEncodedSize(value)` — returns encoded varint size.
- `encodeValue(value, output)` — encodes a `uint32_t` using 7‑bit continuation.
- `decodeValue(input, inputSize, consumedBytes)` — decodes and reports consumed bytes.
- `isLastByte(byte)` — checks if continuation bit is clear.

### Buffer Encoding
- `getEncodedBufferSize(length)` — computes maximum output size for a buffer.
- `encodeBuffer(input, inLen, output)` — packs raw bytes into 7‑bit septets.
- `decodeBuffer(input, inLen, output, outLen)` — unpacks septets back to original bytes.

### Masks
- `leftMask(n)` — returns a mask for the lowest `n` bits.

## Example (Integer Encoding)

```cpp
uint8_t out[5];
SevenBitEncoding::encodeValue(300, out);
// out now contains variable-length encoded bytes
```

## Example (Buffer Encoding)

```cpp
uint8_t raw[] = {0xDE, 0xAD, 0xBE, 0xEF};
uint8_t encoded[16];
size_t n = SevenBitEncoding::encodeBuffer(raw, 4, encoded);
```

## License
MIT or your preferred license.

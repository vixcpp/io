# Vix IO

The **io module** provides a simple, modern, and cross-platform API
for working with input and output streams in C++.

It is designed to be:

- simple to use
- lightweight
- consistent
- familiar for developers coming from Node.js, Deno, or Bun

## Philosophy

I/O in Vix follows strict principles:

- simple functions, no boilerplate
- explicit error handling (`Result<T>`)
- stream-first design
- no unnecessary abstractions
- composable APIs

The goal is to bring a **modern developer experience to C++ I/O**.

## Install

Using Vix:

```bash
vix add @vix/io
```

## Quick Start

```cpp
#include <vix/io/Io.hpp>

int main()
{
  auto out = vix::io::stdout_stream();

  vix::io::write_line(out, "Hello Vix IO");
}
```

## Standard Streams

```cpp
auto in  = vix::io::stdin_stream();
auto out = vix::io::stdout_stream();
auto err = vix::io::stderr_stream();
```

## Reading

### Read a line

```cpp
auto line = vix::io::read_line(in);

if (line)
{
  vix::io::write_line(out, line.value());
}
```

### Read all input

```cpp
auto data = vix::io::read_all(in);

if (data)
{
  vix::io::write(out, data.value());
}
```

### Read bytes

```cpp
auto chunk = vix::io::read(in, 1024);
```

## Writing

### Write text

```cpp
vix::io::write(out, "Hello");
vix::io::write_line(out, " world");
```

### Write bytes

```cpp
vix::io::Bytes bytes{65, 66, 67};
vix::io::write(out, bytes);
```

### Flush

```cpp
vix::io::flush(out);
```

## Copy Streams

```cpp
auto result = vix::io::copy(in, out);

if (!result)
{
  vix::io::write_line(err, "copy failed");
}
```

## Buffer

```cpp
vix::io::Buffer buffer;

buffer.append("Hello ");
buffer.append("Buffer");

vix::io::write_line(out, buffer.to_string());
```

## Options

```cpp
vix::io::IoOptions options;
options.chunk_size = 8192;
options.auto_flush = true;
```

Used in:

- read_all()
- write_line()
- copy()

## Error Handling

All operations return a `Result<T>`.

```cpp
auto result = vix::io::read_line(in);

if (!result)
{
  std::cerr << result.error().message() << "\n";
  return;
}
```

## Types

### Bytes

```cpp
using Bytes = std::vector<std::uint8_t>;
```

### Buffer

```cpp
vix::io::Buffer buffer;
buffer.append("data");
```

## Examples

See the `examples/` directory:

- basic.cpp
- stdin.cpp
- stdout.cpp
- read_all.cpp
- buffer.cpp
- copy_stream.cpp
- advanced.cpp

## Design Notes

- `io` is stream-oriented
- `fs` handles filesystem (files, directories)
- `io` handles streams (stdin, stdout, buffers)
- no exceptions required
- simple and composable API

## Why Vix IO?

Traditional C++ I/O:

- verbose
- inconsistent
- hard to compose

Vix IO provides:

- simple APIs
- predictable behavior
- structured errors
- modern developer experience

## License

MIT License


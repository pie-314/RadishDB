# RadishDB

RadishDB is a lightweight, Redis-inspired in-memory key–value database written in C.  
It is built from scratch to understand how real databases work internally, with a focus on data structures, memory management, persistence, and system design.

The project is educational-first, but follows real database architecture principles.

---

## Features

- Key–value store (string to string)
- Hash table with separate chaining for collision handling
- Automatic resizing based on load factor
- Safe insertion, update, and deletion
- Interactive command-line REPL
- Built-in benchmarking command
- Binary persistence using a custom snapshot format (`.rdbx`)
- Clean separation between in-memory logic and persistence layer

---

## Core Design

RadishDB is built around a dynamically resizing hash table:

- Buckets are implemented as linked lists
- Load factor threshold is 0.75
- Rehashing redistributes all entries during resize
- Keys and values are heap-allocated strings
- Persistence serializes raw key/value bytes using length-prefix encoding

The persistence format is binary, versioned, and self-identifying via a magic header.

---

## Project Structure

```

src/
├── main.c           # REPL and command handling
├── hashtable.c      # Core hash table implementation
├── hashtable.h
├── persistence.c    # SAVE / LOAD (.rdbx format)
├── persistence.h
└── Makefile

```

---

## Building

Requires a C compiler such as GCC or Clang.

```

make

```

Run the database:

```

./radishdb

```

---

## REPL Commands

### Basic operations

```

SET key value
GET key
DEL key

```

### Benchmarking

```

BENCH 10000

```

Inserts 10,000 generated key–value pairs (`key0 → val0`, etc.) to test performance and resizing.

### Persistence

```

SAVE filename
LOAD filename

```

RadishDB snapshot files use the `.rdbx` extension.

---

## `.rdbx` File Format

Each snapshot file has the following binary layout:

```

[RDBX1]           // magic header + version
[uint32 count]    // number of entries

repeat count times:
[uint32 key_length]
[key bytes]
[uint32 value_length]
[value bytes]

```

The magic header ensures file validity before loading.

---

## Example Session

```

> SET name Pi
> SET lang c
> SAVE data
> FILE SAVED : data.rdbx

(restart program)

> LOAD data.rdbx
> OK
> GET name
> Pi

```

---

## Safety and Debugging

- Header guards prevent multiple definitions
- Binary length-prefixing prevents buffer overruns
- Designed to work with AddressSanitizer and Valgrind
- Snapshot loader validates file format before loading

---

## Roadmap

Planned improvements include:

- Clear-vs-merge behavior on LOAD
- Key expiry (TTL)
- Append-only file (AOF)
- TCP networking
- RESP protocol compatibility
- Event-driven concurrency
- Compression and checksums

---

## Motivation

RadishDB exists to deeply understand:

- Hash tables and resizing
- Manual memory management in C
- Binary serialization
- Persistence mechanisms
- Database design trade-offs

It is not intended to replace Redis, but to learn how Redis-like systems are built.

---

## License

MIT License.

---

## Author

Built as a learning-focused systems project.


# RadishDB

RadishDB is a lightweight, Redis-inspired **in-memory key–value database** written in C.

It is built from scratch as a **systems learning project**, with a focus on:

* data structures
* manual memory management
* persistence mechanisms
* crash recovery
* clean modular design

While educational-first, RadishDB intentionally follows **real database architecture patterns** rather than toy abstractions.

---

## Current Features

### Core storage

* String → string key–value store
* Hash table with **separate chaining**
* Automatic resizing based on load factor
* Safe insertion, update, and deletion
* Heap-managed keys and values

### REPL

* Interactive command-line interface
* Deterministic command parsing
* Clear error handling
* Non-blocking prompt behavior

### Persistence

* **Binary snapshot format** (`.rdbx`)
* **Append-Only File (AOF)** for crash recovery
* AOF replay on startup
* Length-prefixed binary encoding
* Versioned file header with magic bytes

### Tooling

* Built-in benchmarking (`BENCH`)
* Compatible with Valgrind and AddressSanitizer
* Clean module separation (`hashtable`, `persistence`, `aof`, `utils`)

---

## Core Design

RadishDB is built around a dynamically resizing hash table:

* Buckets are implemented as linked lists
* Load factor threshold: **0.75**
* Rehashing redistributes all entries during resize
* Keys and values are heap-allocated (`malloc` / `free`)
* Collision handling via chaining

Persistence is layered on top of the in-memory engine:

1. **Snapshot (.rdbx)**

   * Fast full-state save/load
   * Used for compact persistence

2. **Append-Only File (AOF)**

   * Logs write operations (`SET`, `DEL`)
   * Flushed after each write for crash safety
   * Replayed on startup to reconstruct state

Replay is deterministic and silent, following WAL principles.

---

## Project Structure

```
src/
├── main.c           # REPL and command dispatch
├── hashtable.c      # Core hash table logic
├── hashtable.h
├── persistence.c    # Snapshot save/load (.rdbx)
├── persistence.h
├── aof.c            # Append-only file + replay
├── aof.h
├── utils.c          # Shared helpers (tokenizing, trimming)
├── utils.h
└── Makefile
```

---

## Building

Requires a C compiler (GCC or Clang).

```sh
make
```

Run the database:

```sh
./radishdb
```

---

## REPL Commands

### Basic operations

```text
SET key value
GET key
DEL key
COUNT
```

### Benchmarking

```text
BENCH 10000
```

Inserts 10,000 generated key–value pairs (`key0 → value0`, etc.)
Used to test resizing and insertion performance.

### Persistence

```text
SAVE filename
LOAD filename
```

* Snapshot files use the `.rdbx` extension
* AOF is maintained automatically

---

## `.rdbx` Snapshot Format

Binary layout:

```
[RDBX1]            // magic header + version
[uint32 count]     // number of entries

repeat count times:
[uint32 key_len]
[key bytes]
[uint32 value_len]
[value bytes]
```

The magic header is validated before loading to prevent corrupt reads.

---

## Append-Only File (AOF)

* Text-based command log
* Commands appended in execution order
* Flushed after each write (`fflush`)
* Replayed on startup before opening append mode

Example AOF contents:

```text
SET name radish
SET lang c
DEL temp
```

AOF replay reconstructs in-memory state deterministically.

---

## Example Session

```text
> SET name RadishDB
OK
> SET lang c
OK
> SAVE data
OK

(restart program)

> GET name
RadishDB
> COUNT
2
```

---

## Safety and Debugging

* Header guards prevent duplicate symbols
* Strict compilation flags (`-Wall -Wextra -Werror`)
* Designed to be Valgrind- and ASAN-clean
* Length-prefixed encoding prevents buffer overruns
* Replay logic is defensive against malformed input

---

## Roadmap

Planned next steps (in rough order):

* TTL / key expiry
* AOF rewrite / compaction
* Snapshot + AOF interaction rules
* Improved introspection (`INFO`, stats)
* Networking (single-threaded TCP)
* RESP-like protocol
* Background persistence
* Optional durability policies (`fsync` modes)

---

## Motivation

RadishDB exists to **understand how real databases are built**, not to compete with Redis.

The project explores:

* hash table internals
* memory ownership in C
* persistence trade-offs
* write-ahead logging
* crash recovery semantics
* clean module boundaries

---

## License

MIT License.

---

## Author

Built as a systems-learning project to explore database internals in C.


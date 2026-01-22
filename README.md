# RadishDB

RadishDB is a **Redis-inspired, in-memory key–value database** written in **C**, built to understand how real databases work internally.

This project focuses on **storage engine fundamentals**—not frameworks or UI—including:

* write-ahead logging (WAL)
* crash recovery
* expiration & TTL
* log compaction
* protocol-agnostic execution

RadishDB is a **learning-grade but architecturally serious** database, following patterns used in Redis, RocksDB, and PostgreSQL.

---

## Screenshots

### Interactive REPL

![RadishDB REPL](screenshots/repl.png)

### TCP Server Mode

![RadishDB Server](screenshots/server.png)

---

## Core Features

### Key–Value Storage

* String → string mapping
* Hash table with **separate chaining**
* Automatic resizing (load factor > 0.75)
* Heap-managed keys and values
* Safe insert, update, delete

---

### TTL & Expiration

* Per-key expiration timestamps
* Passive expiration on read
* Active expiration via incremental sweeper
* Redis-style TTL semantics

---

### Persistence

RadishDB uses **two complementary persistence layers**.

#### 1️⃣ Snapshot (`.rdbx`)

Compact binary snapshot of the full database state.

**Format:**

```
RDBX1
[uint32 key_count]
repeat:
  [uint32 key_len]
  [key bytes]
  [uint32 value_len]
  [value bytes]
  [time_t expires_at]
```

Used for fast save & restore.

---

#### 2️⃣ Append-Only File (AOF)

Crash-safe write-ahead log.

**Format:**

```
[AOFX1][uint64 base_size]
[uint32 len][ASCII command]
[uint32 len][ASCII command]
...
```

**Properties:**

* Length-prefixed binary framing
* `fsync()` on every write
* Deterministic replay on startup
* Partial writes safely ignored

---

### AOF Rewrite (Compaction)

RadishDB implements Redis-style AOF compaction:

* Serializes current in-memory state
* Writes a fresh AOF
* Discards historical garbage
* Updates growth baseline in header

This guarantees:

* bounded disk growth
* fast startup
* crash safety

---

## Command Engine Architecture

RadishDB is structured around a **protocol-agnostic execution engine**.

```
engine.c   → command semantics
aof.c      → durability (WAL)
expires.c  → TTL & expiration
hashtable  → storage engine
repl.c     → interactive frontend
server.c   → TCP frontend
main.c     → lifecycle & mode selection
```

The same engine powers:

* REPL
* TCP server
* future protocols (RESP, HTTP, etc.)

---

## Supported Commands

```
SET key value
SET key value EX seconds
GET key
DEL key
TTL key
COUNT
SAVE file
LOAD file
BENCH n
INFO
HELP
EXIT
```

---

## Example Session

```
SET name radish
OK
SET color red EX 10
OK
GET name
radish
TTL color
7
```

After restart:

```
GET name
radish
```

State is recovered from the AOF.

---

## Running RadishDB

### Build

```bash
make
```

### REPL mode

```bash
./radishdb
```

### Server mode

```bash
./radishdb --server
```

Server listens on **port 8080** and accepts one client.

---

## Project Structure

```
src/
├── engine.c        # Command execution engine
├── aof.c           # WAL + replay + rewrite
├── expires.c       # TTL & expiration
├── hashtable.c    # Core storage
├── persistence.c  # Snapshot (.rdbx)
├── repl.c         # Interactive shell
├── server.c       # TCP server
├── result.c       # Output formatting
├── utils.c        # Helpers
```

Other files:

* `ARCHITECTURE.md` – design decisions
* `LEARNINGS.md` – what was learned
* `screenshots/` – REPL & server demos

---

## Why RadishDB Is Interesting

RadishDB implements **real database internals**, not toy abstractions:

* write-ahead logging
* crash recovery
* deterministic replay
* TTL correctness
* log compaction
* engine / protocol separation

These are the same problems solved in Redis, RocksDB, PostgreSQL WAL, and Elasticsearch.

---

## Status

**v0.1.0 – Stable learning release**

RadishDB v0.1 represents a **completed storage engine**.
Further work would focus on extensions rather than core correctness.

---

## License

MIT

---

## Author

Built as a **systems-learning project** to understand databases from memory to disk to crash recovery.



# **RadishDB**

RadishDB is a **Redis-inspired, in-memory key–value database** written in **C**, built to deeply understand how real databases work internally.

Instead of focusing on UI or frameworks, RadishDB explores **core database internals** such as:

* storage engine design
* write-ahead logging (WAL)
* crash recovery
* TTL & expiration
* log compaction
* clean engine / protocol separation

This is a **learning-grade database engine**, but it intentionally follows architectural patterns used in real systems like **Redis, RocksDB, and PostgreSQL**.

---

## **Key Features**

### **In-Memory Storage Engine**

* String → string key–value store
* Hash table with **separate chaining**
* Automatic resizing (load factor > 0.75)
* Heap-managed keys and values
* Safe insert, update, and delete

---

### **TTL & Expiration**

* Optional per-key expiration timestamps
* Passive expiration on read
* Active expiration via incremental sweeper
* Redis-style TTL behavior

---

### **Persistence**

RadishDB uses **two complementary persistence layers**.

#### **1. Snapshot (.rdbx)**

A compact binary snapshot representing the full database state.

**Format**

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

Used for:

* fast full-state save
* cold-start recovery

---

#### **2. Append-Only File (AOF / WAL)**

A crash-safe **write-ahead log** recording all mutating commands.

**Format**

```
[AOFX1][uint64 base_size]
[uint32 len][ASCII command]
[uint32 len][ASCII command]
...
```

**Properties**

* Binary length-prefixed framing
* `fsync` on every write
* Deterministic replay on startup
* Partial writes safely ignored
* Crash-safe by construction

---

### **AOF Rewrite (Log Compaction)**

RadishDB implements **AOF rewrite** similar to Redis `BGREWRITEAOF`.

Rewrite behavior:

* serializes current in-memory state
* writes a fresh AOF with a new header
* discards historical garbage
* resets growth baseline

This guarantees:

* bounded disk growth
* faster startup
* long-term durability

---

## **Execution Engine Architecture**

RadishDB is structured around a **protocol-agnostic execution engine**.

```
engine.c    → command semantics
hashtable   → in-memory storage
expires.c   → TTL & expiration
aof.c       → durability, replay, rewrite
result.c    → typed command results
```

Frontends drive the same engine:

* **REPL** (`repl.c`)
* **TCP server** (`server.c`)

No command logic is duplicated.

---

## **Supported Commands**

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
CLEAR
```

---

## **Example Session**

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

State is recovered via AOF replay.

---

## **REPL & TCP Server**

RadishDB supports **two execution modes**.

### **REPL Mode**

```
./radishdb
```

Interactive shell for local use and debugging.

---

### **Server Mode**

```
./radishdb --server
```

Features:

* TCP server (line-based protocol)
* Fork-based multi-client handling
* Shared execution engine with REPL
* One response per command
* Can be tested using `nc`

Example:

```
nc localhost 8080
SET a 1
GET a
```

---

## **Project Structure**

```
src/
├── aof.c / aof.h           # WAL, replay, rewrite
├── engine.c / engine.h    # Command execution engine
├── expires.c / expires.h  # TTL & expiration
├── hashtable.c / .h       # Core in-memory storage
├── persistence.c / .h     # Snapshot (.rdbx)
├── result.c / result.h    # Typed command results
├── repl.c / repl.h        # Interactive REPL
├── server.c / server.h    # TCP server frontend
├── utils.c / utils.h      # Tokenization & helpers
└── main.c                 # Startup & lifecycle
```

Build artifacts live in `build/`.

---

## **Build & Run**

```sh
make
./radishdb
```

---

## **Why RadishDB Is Interesting**

RadishDB implements the **hard parts** of real databases:

* write-ahead logging
* crash recovery
* deterministic replay
* TTL correctness
* log compaction
* format versioning
* engine / protocol separation

These are the same foundations used by Redis, RocksDB, PostgreSQL WAL, and Elasticsearch.

---

## **Roadmap**

Planned future work:

* RESP-like protocol
* Shared-memory or event-driven server
* Typed values (beyond strings)
* Range & prefix scans
* Eviction policies
* Disk-backed data structures
* Transactions

---

## **License**

MIT

---

## **Author**

RadishDB is a **systems-learning project** built to understand database internals, memory ownership, persistence, and crash safety in C.


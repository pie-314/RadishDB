# **RadishDB**

RadishDB is a **Redis-inspired, in-memory key–value database** written in **C**, designed to explore how real databases work internally.

It focuses on **storage-engine fundamentals** such as write-ahead logging, crash recovery, expiration, and log compaction rather than UI or frameworks.

RadishDB is a **learning-grade storage engine** built using the same architectural patterns used by Redis, RocksDB, and PostgreSQL.

---

## **Core Capabilities**

### **Key–Value Storage**

* String → string mapping
* Hash table with separate chaining
* Automatic resizing (load factor > 0.75)
* Heap-managed keys and values
* Safe insert, update, delete

---

### **TTL & Expiration**

* Keys may have expiration timestamps
* Passive expiration on read
* Active expiration via incremental sweeper
* Redis-style expiration behavior

---

### **Persistence**

RadishDB uses two complementary persistence layers.

#### **1. Snapshot (.rdbx)**

A compact binary snapshot of the entire database.

Format:

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

Used for fast full-state save and restore.

---

#### **2. Append-Only File (AOF)**

A crash-safe write-ahead log.

Format:

```
[AOFX1][uint64 base_size]
[uint32 len][ASCII command]
[uint32 len][ASCII command]
...
```

Properties:

* Length-prefixed binary framing
* `fsync` on every write
* Deterministic replay on startup
* Partial writes safely ignored

---

### **AOF Rewrite (Compaction)**

RadishDB implements log compaction similar to Redis `BGREWRITEAOF`.

Rewrite:

* Serializes the current in-memory state
* Writes a fresh AOF with a new header
* Discards historical garbage
* Resets the growth baseline

This guarantees:

* bounded disk growth
* fast startup
* crash safety

The AOF is self-describing via its header.

---

## **Command Engine**

RadishDB is structured around a **protocol-agnostic execution engine**.

```
engine.c   → command semantics
aof.c      → durability
expires.c  → TTL
hashtable  → storage
main.c     → lifecycle + maintenance
```

This allows the same database core to be driven by:

* a REPL
* a TCP server
* future binary protocols

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
```

---

## **Example**

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

After a crash and restart:

```
GET name
radish
```

State is recovered from the AOF.

---

## **Why RadishDB is Interesting**

RadishDB implements the same difficult pieces found in production databases:

• write-ahead logging
• crash recovery
• deterministic replay
• TTL correctness
• compaction
• format evolution
• engine / protocol separation

These are the foundations behind Redis, RocksDB, PostgreSQL WAL, and Elasticsearch segments.

---

## **Project Structure**

```
src/
├── engine.c        # Command execution engine
├── aof.c           # WAL + replay + rewrite
├── expires.c       # TTL & expiration
├── hashtable.c    # Core storage
├── persistence.c  # RDBX snapshot
├── utils.c        # Tokenization, helpers
├── main.c         # Lifecycle & maintenance
```

Build artifacts live in `build/`
Data files live in `data/`

---

## **Build & Run**

```sh
make
./radishdb
```

---

## **Roadmap**

Planned next steps:

* TCP server
* RESP-like protocol
* Typed values
* Prefix & range scans
* Eviction policies
* Disk-backed index structures
* Transactions

---

## **License**

MIT

---

## **Author**

RadishDB is built as a **systems-learning project** to explore database internals from memory to disk to crash recovery.


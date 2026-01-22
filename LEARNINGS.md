# RadishDB – Learnings

## What I learned
- How write-ahead logging guarantees crash safety
- Why length-prefixed logs matter
- How TTL interacts with persistence
- How separating engine and protocol simplifies everything
- How small ownership mistakes cause segfaults

## Bugs that taught me the most
- Use-after-free during snapshot load
- Missing AOF DEL causing incorrect recovery
- Header handling in AOF rewrite

## Design tradeoffs
- Single-threaded by design
- No priority queue for TTL
- No clustering or SQL

## What I intentionally did NOT build
- Threads
- Replication
- Advanced indexing


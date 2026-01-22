# Architecture

RadishDB is split into a protocol-agnostic engine and multiple frontends.

engine.c
- Parses commands
- Executes semantics
- Returns Result (no IO)

repl.c / server.c
- Handle input/output
- Call engine
- Format Result

Persistence:
- AOF = write-ahead log
- RDBX = snapshot


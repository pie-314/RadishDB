# Build 
FROM gcc:13 AS builder
WORKDIR /build
COPY src ./src
COPY Makefile .
RUN make

# Runtime 
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=builder /build/radishdb .
RUN mkdir -p /app/aof
EXPOSE 6379
VOLUME ["/app/aof"]
ENTRYPOINT ["./radishdb"]
CMD ["--server"]


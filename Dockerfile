# ============================================
# Chennai Lang Online Compiler — Dockerfile
# Multi-stage build: Compile C++ + Java → Run Node.js
# v2.0 — Java + C++ Hybrid Compiler with NASM backend
# ============================================

# --- Stage 1: Build the Chennai Lang C++ compiler binary ---
FROM gcc:13 AS builder

WORKDIR /build
COPY src/ ./src/

# Compile the hybrid compiler (includes codegen + token_reader)
RUN g++ -std=c++17 -O2 -static -o chennai_lang \
    src/main.cpp src/lexer.cpp src/parser.cpp src/interpreter.cpp \
    src/codegen.cpp src/token_reader.cpp \
    -Isrc

# --- Stage 2: Build the Java Lexer ---
FROM eclipse-temurin:17-jdk-jammy AS java-builder

WORKDIR /build
COPY src/java/ ./src/java/

RUN mkdir -p /build/java_classes && \
    javac -d /build/java_classes \
    src/java/com/chennai/lexer/TokenType.java \
    src/java/com/chennai/lexer/Token.java \
    src/java/com/chennai/lexer/ChennaiLexer.java

# --- Stage 3: Node.js runtime ---
FROM node:20-slim

# Install Java runtime for the Java lexer
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    openjdk-17-jre-headless \
    nasm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the compiled C++ binary from builder
COPY --from=builder /build/chennai_lang /app/chennai_lang
RUN chmod +x /app/chennai_lang

# Copy Java lexer classes from java-builder
COPY --from=java-builder /build/java_classes /app/java_classes

# Copy example programs
COPY examples/ /app/examples/

# Copy build scripts
COPY build.bat chennai.bat test_compile.bat /app/

# Copy web application
COPY web/package.json /app/web/
WORKDIR /app/web
RUN npm install --production

COPY web/server.js /app/web/
COPY web/public/ /app/web/public/

# Set environment variables
ENV CHENNAI_LANG_BIN=/app/chennai_lang
ENV JAVA_LEXER_CP=/app/java_classes
ENV PORT=3000

EXPOSE 3000

CMD ["node", "server.js"]

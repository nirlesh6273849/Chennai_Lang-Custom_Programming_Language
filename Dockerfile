# ============================================
# Chennai Lang Online Compiler & Interpreter
# Legacy Interpreter Dockerfile (Fast & Lightweight)
# ============================================

# Stage 1: Build the Chennai Lang C++ binary
FROM gcc:13 AS builder

WORKDIR /build
COPY src/ ./src/

RUN g++ -std=c++17 -O2 -static -o chennai_lang \
    src/main.cpp src/lexer.cpp src/parser.cpp src/interpreter.cpp \
    src/codegen.cpp src/token_reader.cpp \
    -Isrc

# Stage 2: Node.js runtime
FROM node:20-slim

WORKDIR /app

# Copy compiled C++ binary
COPY --from=builder /build/chennai_lang /app/chennai_lang
RUN chmod +x /app/chennai_lang

# Copy example programs
COPY examples/ /app/examples/

# Copy web application
COPY web/package.json /app/web/
WORKDIR /app/web
RUN npm install --production

COPY web/server.js /app/web/
COPY web/public/ /app/web/public/

# Environment variables
ENV CHENNAI_LANG_BIN=/app/chennai_lang
ENV PORT=3000

EXPOSE 3000

CMD ["node", "server.js"]


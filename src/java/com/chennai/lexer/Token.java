package com.chennai.lexer;

/**
 * Represents a single lexical token from Chennai Lang source code.
 */
public class Token {
    public final TokenType type;
    public final String lexeme;
    public final int line;
    public final int col;

    public Token(TokenType type, String lexeme, int line, int col) {
        this.type = type;
        this.lexeme = lexeme;
        this.line = line;
        this.col = col;
    }

    /**
     * Serialize this token to a JSON object string.
     * Hand-rolled to avoid external dependencies.
     */
    public String toJson() {
        StringBuilder sb = new StringBuilder();
        sb.append("{\"type\":\"").append(type.name()).append("\"");
        sb.append(",\"lexeme\":\"").append(escapeJson(lexeme)).append("\"");
        sb.append(",\"line\":").append(line);
        sb.append(",\"col\":").append(col);
        sb.append("}");
        return sb.toString();
    }

    /**
     * Escape special characters for JSON string output.
     */
    private static String escapeJson(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"':  sb.append("\\\""); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                case '\0': sb.append("\\u0000"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int) c));
                    } else {
                        sb.append(c);
                    }
                    break;
            }
        }
        return sb.toString();
    }

    @Override
    public String toString() {
        return type.name() + "(\"" + lexeme + "\") @" + line + ":" + col;
    }
}

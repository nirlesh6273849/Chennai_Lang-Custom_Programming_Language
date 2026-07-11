package com.chennai.lexer;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Chennai Lang Lexer — Java implementation.
 * 
 * Reads a .ch source file and outputs a JSON array of tokens to stdout.
 * This is a faithful port of the C++ lexer (src/lexer.cpp).
 * 
 * Usage: java com.chennai.lexer.ChennaiLexer <filename.ch>
 */
public class ChennaiLexer {

    // ========================================================================
    // Keyword table
    // ========================================================================

    private static final Map<String, TokenType> KEYWORDS = new HashMap<>();
    static {
        KEYWORDS.put("polamanna",   TokenType.TOK_POLAMANNA);
        KEYWORDS.put("niruthuanna", TokenType.TOK_NIRUTHUANNA);
        KEYWORDS.put("Op",          TokenType.TOK_OP);
        KEYWORDS.put("sollu",       TokenType.TOK_SOLLU);
        KEYWORDS.put("if",          TokenType.TOK_IF);
        KEYWORDS.put("elif",        TokenType.TOK_ELIF);
        KEYWORDS.put("else",        TokenType.TOK_ELSE);
        KEYWORDS.put("while",       TokenType.TOK_WHILE);
        KEYWORDS.put("main",        TokenType.TOK_MAIN);
        KEYWORDS.put("int",         TokenType.TOK_INT);
        KEYWORDS.put("float",       TokenType.TOK_FLOAT);
        KEYWORDS.put("char",        TokenType.TOK_CHAR);
        KEYWORDS.put("string",      TokenType.TOK_STRING);
        KEYWORDS.put("LHS",         TokenType.TOK_LHS);
        KEYWORDS.put("RHS",         TokenType.TOK_RHS);
        KEYWORDS.put("operator",    TokenType.TOK_OPERATOR);
    }

    // ========================================================================
    // Instance state
    // ========================================================================

    private final String src;
    private int pos;
    private int line;
    private int col;

    public ChennaiLexer(String source) {
        this.src = source;
        this.pos = 0;
        this.line = 1;
        this.col = 1;
    }

    // ========================================================================
    // Character helpers
    // ========================================================================

    private char current() {
        if (pos >= src.length()) return '\0';
        return src.charAt(pos);
    }

    private char peek() {
        if (pos + 1 >= src.length()) return '\0';
        return src.charAt(pos + 1);
    }

    private char advance() {
        char c = current();
        pos++;
        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        return c;
    }

    // ========================================================================
    // Whitespace & comments
    // ========================================================================

    private void skipWhitespace() {
        while (pos < src.length()) {
            char c = current();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance();
            } else if (c == '/' && peek() == '/') {
                skipLineComment();
            } else {
                break;
            }
        }
    }

    private void skipLineComment() {
        while (pos < src.length() && current() != '\n') {
            advance();
        }
    }

    // ========================================================================
    // Token factories
    // ========================================================================

    private Token makeToken(TokenType type, String lexeme) {
        return new Token(type, lexeme, line, col);
    }

    private Token readNumber() {
        int startCol = col;
        StringBuilder num = new StringBuilder();
        while (pos < src.length() && Character.isDigit(current())) {
            num.append(advance());
        }
        // Check for float
        if (current() == '.' && Character.isDigit(peek())) {
            num.append(advance()); // consume '.'
            while (pos < src.length() && Character.isDigit(current())) {
                num.append(advance());
            }
            return new Token(TokenType.TOK_FLOAT_LIT, num.toString(), line, startCol);
        }
        return new Token(TokenType.TOK_NUMBER, num.toString(), line, startCol);
    }

    private Token readString() {
        int startCol = col;
        advance(); // consume opening quote
        StringBuilder str = new StringBuilder();
        while (pos < src.length() && current() != '"') {
            if (current() == '\\') {
                advance();
                char esc = advance();
                switch (esc) {
                    case 'n':  str.append('\n'); break;
                    case 't':  str.append('\t'); break;
                    case '\\': str.append('\\'); break;
                    case '"':  str.append('"');  break;
                    default:   str.append(esc);  break;
                }
            } else {
                str.append(advance());
            }
        }
        if (pos >= src.length()) {
            throw new RuntimeException("Unterminated string at line " + line);
        }
        advance(); // consume closing quote
        return new Token(TokenType.TOK_STRING_LIT, str.toString(), line, startCol);
    }

    private Token readChar() {
        int startCol = col;
        advance(); // consume opening quote
        String ch;
        if (current() == '\\') {
            advance();
            char esc = advance();
            switch (esc) {
                case 'n':  ch = "\n"; break;
                case 't':  ch = "\t"; break;
                case '\\': ch = "\\"; break;
                case '\'': ch = "'";  break;
                default:   ch = String.valueOf(esc); break;
            }
        } else {
            ch = String.valueOf(advance());
        }
        if (current() != '\'') {
            throw new RuntimeException("Unterminated char literal at line " + line);
        }
        advance(); // consume closing quote
        return new Token(TokenType.TOK_CHAR_LIT, ch, line, startCol);
    }

    private Token readIdentifierOrKeyword() {
        int startCol = col;
        StringBuilder id = new StringBuilder();
        while (pos < src.length() && (Character.isLetterOrDigit(current()) || current() == '_')) {
            id.append(advance());
        }

        String word = id.toString();

        // Check for array types: "int[]", "float[]", "char[]"
        if ((word.equals("int") || word.equals("float") || word.equals("char"))
                && current() == '[' && peek() == ']') {
            advance(); // consume [
            advance(); // consume ]
            if (word.equals("int"))   return new Token(TokenType.TOK_INT_ARR,   "int[]",   line, startCol);
            if (word.equals("float")) return new Token(TokenType.TOK_FLOAT_ARR, "float[]", line, startCol);
            if (word.equals("char"))  return new Token(TokenType.TOK_CHAR_ARR,  "char[]",  line, startCol);
        }

        // Check keywords
        TokenType kwType = KEYWORDS.get(word);
        if (kwType != null) {
            return new Token(kwType, word, line, startCol);
        }

        return new Token(TokenType.TOK_IDENTIFIER, word, line, startCol);
    }

    // ========================================================================
    // Main tokenizer
    // ========================================================================

    private Token nextToken() {
        skipWhitespace();
        if (pos >= src.length()) {
            return makeToken(TokenType.TOK_EOF, "");
        }

        int startCol = col;
        char c = current();

        // Numbers
        if (Character.isDigit(c)) return readNumber();

        // String literal
        if (c == '"') return readString();

        // Char literal
        if (c == '\'') return readChar();

        // Identifiers / keywords
        if (Character.isLetter(c) || c == '_') return readIdentifierOrKeyword();

        // Two-character operators
        if (c == '<' && peek() == '=') { advance(); advance(); return new Token(TokenType.TOK_LE, "<=", line, startCol); }
        if (c == '>' && peek() == '=') { advance(); advance(); return new Token(TokenType.TOK_GE, ">=", line, startCol); }
        if (c == '=' && peek() == '=') { advance(); advance(); return new Token(TokenType.TOK_EQ, "==", line, startCol); }
        if (c == '!' && peek() == '=') { advance(); advance(); return new Token(TokenType.TOK_NE, "!=", line, startCol); }
        if (c == '&' && peek() == '&') { advance(); advance(); return new Token(TokenType.TOK_AND, "&&", line, startCol); }
        if (c == '|' && peek() == '|') { advance(); advance(); return new Token(TokenType.TOK_OR, "||", line, startCol); }
        if (c == ':' && peek() == '=') { advance(); advance(); return new Token(TokenType.TOK_COLON_ASSIGN, ":=", line, startCol); }

        // Single-character tokens
        advance();
        switch (c) {
            case '+': return new Token(TokenType.TOK_PLUS,      "+", line, startCol);
            case '-': return new Token(TokenType.TOK_MINUS,     "-", line, startCol);
            case '*': return new Token(TokenType.TOK_STAR,      "*", line, startCol);
            case '/': return new Token(TokenType.TOK_SLASH,     "/", line, startCol);
            case '%': return new Token(TokenType.TOK_PERCENT,   "%", line, startCol);
            case '!': return new Token(TokenType.TOK_BANG,      "!", line, startCol);
            case '<': return new Token(TokenType.TOK_LT,        "<", line, startCol);
            case '>': return new Token(TokenType.TOK_GT,        ">", line, startCol);
            case '=': return new Token(TokenType.TOK_ASSIGN,    "=", line, startCol);
            case '(': return new Token(TokenType.TOK_LPAREN,    "(", line, startCol);
            case ')': return new Token(TokenType.TOK_RPAREN,    ")", line, startCol);
            case '{': return new Token(TokenType.TOK_LBRACE,    "{", line, startCol);
            case '}': return new Token(TokenType.TOK_RBRACE,    "}", line, startCol);
            case '[': return new Token(TokenType.TOK_LBRACKET,  "[", line, startCol);
            case ']': return new Token(TokenType.TOK_RBRACKET,  "]", line, startCol);
            case ',': return new Token(TokenType.TOK_COMMA,     ",", line, startCol);
            case ';': return new Token(TokenType.TOK_SEMICOLON, ";", line, startCol);
            case ':': return new Token(TokenType.TOK_COLON,     ":", line, startCol);
            case '^': return new Token(TokenType.TOK_CARET,     "^", line, startCol);
            default:
                throw new RuntimeException("Unexpected character '" + c 
                    + "' at line " + line + " col " + startCol);
        }
    }

    /**
     * Tokenize the entire source and return a list of tokens.
     */
    public List<Token> tokenize() {
        List<Token> tokens = new ArrayList<>();
        while (true) {
            Token tok = nextToken();
            tokens.add(tok);
            if (tok.type == TokenType.TOK_EOF) break;
        }
        return tokens;
    }

    // ========================================================================
    // Entry point — reads .ch file, outputs JSON token array to stdout
    // ========================================================================

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Chennai Lang Java Lexer v1.0");
            System.err.println("Usage: java com.chennai.lexer.ChennaiLexer <filename.ch>");
            System.exit(1);
        }

        String filename = args[0];
        String source;
        try {
            source = new String(Files.readAllBytes(Paths.get(filename)));
        } catch (IOException e) {
            System.err.println("Error: Cannot open file '" + filename + "'");
            System.exit(1);
            return;
        }

        try {
            ChennaiLexer lexer = new ChennaiLexer(source);
            List<Token> tokens = lexer.tokenize();

            // Output as JSON array
            StringBuilder json = new StringBuilder();
            json.append("[\n");
            for (int i = 0; i < tokens.size(); i++) {
                json.append("  ").append(tokens.get(i).toJson());
                if (i < tokens.size() - 1) {
                    json.append(",");
                }
                json.append("\n");
            }
            json.append("]");

            System.out.println(json.toString());

        } catch (RuntimeException e) {
            System.err.println("Lexer Error: " + e.getMessage());
            System.exit(1);
        }
    }
}

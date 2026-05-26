// ============================================================================
// Multi-Target Source-to-Source Compiler / Transpiler
// Architecture: Lexer -> Parser -> AST -> Semantic Analysis -> IR -> Optimizer
//               -> Code Generation (Python / C++ / Java)
// ============================================================================
#include <bits/stdc++.h>
using namespace std;

// ============================================================================
// SECTION 1: TOKEN TYPES
// ============================================================================

enum TokenType {
    // Literals
    TOK_NUMBER, TOK_FLOAT_LIT, TOK_STRING_LIT, TOK_CHAR_LIT,
    // Identifiers
    TOK_IDENTIFIER,
    // Keywords - declarations
    TOK_LET, TOK_INT, TOK_FLOAT, TOK_DOUBLE, TOK_STRING, TOK_BOOL,
    TOK_CHAR, TOK_VOID, TOK_AUTO,
    // Keywords - values
    TOK_TRUE, TOK_FALSE, TOK_NULL_KW,
    // Keywords - control flow
    TOK_IF, TOK_ELSE, TOK_SWITCH, TOK_CASE, TOK_DEFAULT,
    TOK_FOR, TOK_WHILE, TOK_DO,
    TOK_BREAK, TOK_CONTINUE, TOK_RETURN,
    // Keywords - functions/classes
    TOK_FUNCTION, TOK_CLASS, TOK_NEW, TOK_THIS,
    TOK_PUBLIC, TOK_PRIVATE, TOK_PROTECTED, TOK_STATIC,
    TOK_EXTENDS, TOK_IMPLEMENTS, TOK_INTERFACE, TOK_ABSTRACT, TOK_VIRTUAL,
    // Keywords - exceptions
    TOK_TRY, TOK_CATCH, TOK_FINALLY, TOK_THROW,
    // Keywords - other
    TOK_IMPORT, TOK_INCLUDE, TOK_PRINT, TOK_INPUT,
    // Operators - arithmetic
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    // Operators - assignment
    TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN,
    TOK_STAR_ASSIGN, TOK_SLASH_ASSIGN,
    // Operators - comparison
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,
    // Operators - logical
    TOK_AND, TOK_OR, TOK_NOT,
    // Operators - inc/dec
    TOK_INCREMENT, TOK_DECREMENT,
    // Operators - member access
    TOK_DOT, TOK_ARROW, TOK_SCOPE,
    // Delimiters
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_SEMICOLON, TOK_COMMA, TOK_COLON, TOK_QUESTION,
    // Other
    TOK_AMPERSAND, TOK_HASH,
    // End
    TOK_EOF
};

struct Token {
    TokenType type;
    string value;
    int line;
    int col;
};

// ============================================================================
// SECTION 2: LEXER
// ============================================================================

class Lexer {
    string text;
    int pos = 0;
    int line = 1;
    int col = 1;

    char cur() { return pos < (int)text.size() ? text[pos] : '\0'; }
    char peekChar(int off = 1) {
        int p = pos + off;
        return p < (int)text.size() ? text[p] : '\0';
    }
    void adv() {
        if (cur() == '\n') { line++; col = 1; }
        else { col++; }
        pos++;
    }

    void skipWhitespace() {
        while (pos < (int)text.size() && isspace(cur())) adv();
    }

    void skipLineComment() {
        while (pos < (int)text.size() && cur() != '\n') adv();
    }

    void skipBlockComment() {
        adv(); adv(); // skip /*
        while (pos < (int)text.size()) {
            if (cur() == '*' && peekChar() == '/') { adv(); adv(); return; }
            adv();
        }
    }

    Token makeToken(TokenType t, string v) {
        return {t, v, line, col};
    }

    // Keyword map
    static unordered_map<string, TokenType>& keywords() {
        static unordered_map<string, TokenType> kw = {
            {"let", TOK_LET}, {"int", TOK_INT}, {"float", TOK_FLOAT},
            {"double", TOK_DOUBLE}, {"string", TOK_STRING}, {"bool", TOK_BOOL},
            {"char", TOK_CHAR}, {"void", TOK_VOID}, {"auto", TOK_AUTO},
            {"true", TOK_TRUE}, {"false", TOK_FALSE}, {"null", TOK_NULL_KW},
            {"nullptr", TOK_NULL_KW},
            {"if", TOK_IF}, {"else", TOK_ELSE},
            {"switch", TOK_SWITCH}, {"case", TOK_CASE}, {"default", TOK_DEFAULT},
            {"for", TOK_FOR}, {"while", TOK_WHILE}, {"do", TOK_DO},
            {"break", TOK_BREAK}, {"continue", TOK_CONTINUE}, {"return", TOK_RETURN},
            {"function", TOK_FUNCTION}, {"class", TOK_CLASS},
            {"new", TOK_NEW}, {"this", TOK_THIS},
            {"public", TOK_PUBLIC}, {"private", TOK_PRIVATE},
            {"protected", TOK_PROTECTED}, {"static", TOK_STATIC},
            {"extends", TOK_EXTENDS}, {"implements", TOK_IMPLEMENTS},
            {"interface", TOK_INTERFACE}, {"abstract", TOK_ABSTRACT},
            {"virtual", TOK_VIRTUAL},
            {"try", TOK_TRY}, {"catch", TOK_CATCH},
            {"finally", TOK_FINALLY}, {"throw", TOK_THROW},
            {"import", TOK_IMPORT}, {"include", TOK_INCLUDE},
            {"print", TOK_PRINT}, {"input", TOK_INPUT},
            // C++ type aliases
            {"boolean", TOK_BOOL}, {"String", TOK_STRING},
        };
        return kw;
    }

public:
    Lexer(const string& input) : text(input) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (pos < (int)text.size()) {
            skipWhitespace();
            if (pos >= (int)text.size()) break;

            int startLine = line, startCol = col;

            // Comments
            if (cur() == '/' && peekChar() == '/') { skipLineComment(); continue; }
            if (cur() == '/' && peekChar() == '*') { skipBlockComment(); continue; }

            // String literals
            if (cur() == '"') {
                adv();
                string s = "";
                while (pos < (int)text.size() && cur() != '"') {
                    if (cur() == '\\') {
                        adv();
                        switch (cur()) {
                            case 'n': s += '\n'; break;
                            case 't': s += '\t'; break;
                            case '\\': s += '\\'; break;
                            case '"': s += '"'; break;
                            default: s += cur(); break;
                        }
                    } else {
                        s += cur();
                    }
                    adv();
                }
                if (cur() == '"') adv();
                tokens.push_back({TOK_STRING_LIT, s, startLine, startCol});
                continue;
            }

            // Char literals
            if (cur() == '\'') {
                adv();
                string s = "";
                if (cur() == '\\') {
                    adv();
                    switch (cur()) {
                        case 'n': s = "\\n"; break;
                        case 't': s = "\\t"; break;
                        case '\\': s = "\\\\"; break;
                        case '\'': s = "'"; break;
                        default: s += cur(); break;
                    }
                    adv();
                } else {
                    s += cur();
                    adv();
                }
                if (cur() == '\'') adv();
                tokens.push_back({TOK_CHAR_LIT, s, startLine, startCol});
                continue;
            }

            // Numbers (int and float)
            if (isdigit(cur())) {
                string num = "";
                while (pos < (int)text.size() && isdigit(cur())) { num += cur(); adv(); }
                if (cur() == '.' && isdigit(peekChar())) {
                    num += cur(); adv();
                    while (pos < (int)text.size() && isdigit(cur())) { num += cur(); adv(); }
                    tokens.push_back({TOK_FLOAT_LIT, num, startLine, startCol});
                } else {
                    tokens.push_back({TOK_NUMBER, num, startLine, startCol});
                }
                continue;
            }

            // Identifiers and keywords
            if (isalpha(cur()) || cur() == '_') {
                string id = "";
                while (pos < (int)text.size() && (isalnum(cur()) || cur() == '_')) {
                    id += cur(); adv();
                }
                auto& kw = keywords();
                auto it = kw.find(id);
                if (it != kw.end()) {
                    tokens.push_back({it->second, id, startLine, startCol});
                } else {
                    tokens.push_back({TOK_IDENTIFIER, id, startLine, startCol});
                }
                continue;
            }

            // Two-character operators (check before single-char)
            if (cur() == '=' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_EQ, "==", startLine, startCol}); continue; }
            if (cur() == '!' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_NEQ, "!=", startLine, startCol}); continue; }
            if (cur() == '<' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_LTE, "<=", startLine, startCol}); continue; }
            if (cur() == '>' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_GTE, ">=", startLine, startCol}); continue; }
            if (cur() == '&' && peekChar() == '&') { adv(); adv(); tokens.push_back({TOK_AND, "&&", startLine, startCol}); continue; }
            if (cur() == '|' && peekChar() == '|') { adv(); adv(); tokens.push_back({TOK_OR, "||", startLine, startCol}); continue; }
            if (cur() == '+' && peekChar() == '+') { adv(); adv(); tokens.push_back({TOK_INCREMENT, "++", startLine, startCol}); continue; }
            if (cur() == '-' && peekChar() == '-') { adv(); adv(); tokens.push_back({TOK_DECREMENT, "--", startLine, startCol}); continue; }
            if (cur() == '-' && peekChar() == '>') { adv(); adv(); tokens.push_back({TOK_ARROW, "->", startLine, startCol}); continue; }
            if (cur() == ':' && peekChar() == ':') { adv(); adv(); tokens.push_back({TOK_SCOPE, "::", startLine, startCol}); continue; }
            if (cur() == '+' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_PLUS_ASSIGN, "+=", startLine, startCol}); continue; }
            if (cur() == '-' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_MINUS_ASSIGN, "-=", startLine, startCol}); continue; }
            if (cur() == '*' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_STAR_ASSIGN, "*=", startLine, startCol}); continue; }
            if (cur() == '/' && peekChar() == '=') { adv(); adv(); tokens.push_back({TOK_SLASH_ASSIGN, "/=", startLine, startCol}); continue; }

            // Single-character operators and delimiters
            switch (cur()) {
                case '+': adv(); tokens.push_back({TOK_PLUS, "+", startLine, startCol}); continue;
                case '-': adv(); tokens.push_back({TOK_MINUS, "-", startLine, startCol}); continue;
                case '*': adv(); tokens.push_back({TOK_STAR, "*", startLine, startCol}); continue;
                case '/': adv(); tokens.push_back({TOK_SLASH, "/", startLine, startCol}); continue;
                case '%': adv(); tokens.push_back({TOK_PERCENT, "%", startLine, startCol}); continue;
                case '=': adv(); tokens.push_back({TOK_ASSIGN, "=", startLine, startCol}); continue;
                case '<': adv(); tokens.push_back({TOK_LT, "<", startLine, startCol}); continue;
                case '>': adv(); tokens.push_back({TOK_GT, ">", startLine, startCol}); continue;
                case '!': adv(); tokens.push_back({TOK_NOT, "!", startLine, startCol}); continue;
                case '(': adv(); tokens.push_back({TOK_LPAREN, "(", startLine, startCol}); continue;
                case ')': adv(); tokens.push_back({TOK_RPAREN, ")", startLine, startCol}); continue;
                case '{': adv(); tokens.push_back({TOK_LBRACE, "{", startLine, startCol}); continue;
                case '}': adv(); tokens.push_back({TOK_RBRACE, "}", startLine, startCol}); continue;
                case '[': adv(); tokens.push_back({TOK_LBRACKET, "[", startLine, startCol}); continue;
                case ']': adv(); tokens.push_back({TOK_RBRACKET, "]", startLine, startCol}); continue;
                case ';': adv(); tokens.push_back({TOK_SEMICOLON, ";", startLine, startCol}); continue;
                case ',': adv(); tokens.push_back({TOK_COMMA, ",", startLine, startCol}); continue;
                case ':': adv(); tokens.push_back({TOK_COLON, ":", startLine, startCol}); continue;
                case '?': adv(); tokens.push_back({TOK_QUESTION, "?", startLine, startCol}); continue;
                case '.': adv(); tokens.push_back({TOK_DOT, ".", startLine, startCol}); continue;
                case '&': adv(); tokens.push_back({TOK_AMPERSAND, "&", startLine, startCol}); continue;
                case '#': {
                    adv();
                    // Handle #include
                    string directive = "";
                    while (pos < (int)text.size() && isalpha(cur())) { directive += cur(); adv(); }
                    if (directive == "include") {
                        skipWhitespace();
                        string path = "";
                        char delim = cur();
                        if (delim == '<' || delim == '"') {
                            char endDelim = (delim == '<') ? '>' : '"';
                            adv();
                            while (pos < (int)text.size() && cur() != endDelim) { path += cur(); adv(); }
                            if (cur() == endDelim) adv();
                        }
                        tokens.push_back({TOK_INCLUDE, path, startLine, startCol});
                    } else {
                        tokens.push_back({TOK_HASH, "#" + directive, startLine, startCol});
                    }
                    continue;
                }
            }

            // Skip unknown characters
            adv();
        }
        tokens.push_back({TOK_EOF, "", line, col});
        return tokens;
    }
};

// ============================================================================
// SECTION 3: AST NODES
// ============================================================================

// Forward declarations
class Expr;
class Stmt;
using ExprPtr = shared_ptr<Expr>;
using StmtPtr = shared_ptr<Stmt>;

// --- Base nodes ---
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class Expr : public ASTNode {};
class Stmt : public ASTNode {};

// --- Expression Nodes ---

class NumberLiteral : public Expr {
public:
    int value;
    NumberLiteral(int v) : value(v) {}
};

class FloatLiteral : public Expr {
public:
    double value;
    FloatLiteral(double v) : value(v) {}
};

class StringLiteral : public Expr {
public:
    string value;
    StringLiteral(string v) : value(v) {}
};

class BoolLiteral : public Expr {
public:
    bool value;
    BoolLiteral(bool v) : value(v) {}
};

class CharLiteral : public Expr {
public:
    string value;
    CharLiteral(string v) : value(v) {}
};

class NullLiteral : public Expr {};

class Variable : public Expr {
public:
    string name;
    Variable(string n) : name(n) {}
};

class ThisExpr : public Expr {};

class BinaryExpr : public Expr {
public:
    ExprPtr left;
    string op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, string o, ExprPtr r) : left(l), op(o), right(r) {}
};

class UnaryExpr : public Expr {
public:
    string op;
    ExprPtr operand;
    bool isPrefix; // true = prefix (!x, -x, ++x), false = postfix (x++, x--)
    UnaryExpr(string o, ExprPtr e, bool prefix) : op(o), operand(e), isPrefix(prefix) {}
};

class AssignExpr : public Expr {
public:
    ExprPtr target;
    string op; // =, +=, -=, *=, /=
    ExprPtr value;
    AssignExpr(ExprPtr t, string o, ExprPtr v) : target(t), op(o), value(v) {}
};

class TernaryExpr : public Expr {
public:
    ExprPtr condition;
    ExprPtr thenExpr;
    ExprPtr elseExpr;
    TernaryExpr(ExprPtr c, ExprPtr t, ExprPtr e) : condition(c), thenExpr(t), elseExpr(e) {}
};

class CallExpr : public Expr {
public:
    ExprPtr callee;
    vector<ExprPtr> args;
    CallExpr(ExprPtr c, vector<ExprPtr> a) : callee(c), args(a) {}
};

class IndexExpr : public Expr {
public:
    ExprPtr object;
    ExprPtr index;
    IndexExpr(ExprPtr o, ExprPtr i) : object(o), index(i) {}
};

class MemberExpr : public Expr {
public:
    ExprPtr object;
    string member;
    MemberExpr(ExprPtr o, string m) : object(o), member(m) {}
};

class NewExpr : public Expr {
public:
    string className;
    vector<ExprPtr> args;
    NewExpr(string c, vector<ExprPtr> a) : className(c), args(a) {}
};

class ArrayLitExpr : public Expr {
public:
    vector<ExprPtr> elements;
    ArrayLitExpr(vector<ExprPtr> e) : elements(e) {}
};

class ArrayAccessExpr : public Expr {
public:
    ExprPtr array;
    ExprPtr index;
    ArrayAccessExpr(ExprPtr a, ExprPtr i) : array(a), index(i) {}
};

class InputExpr : public Expr {
public:
    ExprPtr prompt; // optional
    InputExpr(ExprPtr p = nullptr) : prompt(p) {}
};

class CastExpr : public Expr {
public:
    string targetType;
    ExprPtr expr;
    CastExpr(string t, ExprPtr e) : targetType(t), expr(e) {}
};

// --- Statement Nodes ---

class ExprStmt : public Stmt {
public:
    ExprPtr expr;
    ExprStmt(ExprPtr e) : expr(e) {}
};

class VarDeclStmt : public Stmt {
public:
    string type; // "auto" for let, "int", "float", "double", "string", "bool", "char", etc.
    string name;
    ExprPtr init; // can be null
    bool isArray;
    VarDeclStmt(string t, string n, ExprPtr i, bool arr = false)
        : type(t), name(n), init(i), isArray(arr) {}
};

class BlockStmt : public Stmt {
public:
    vector<StmtPtr> stmts;
    BlockStmt(vector<StmtPtr> s) : stmts(s) {}
};

class PrintStmt : public Stmt {
public:
    vector<ExprPtr> args;
    PrintStmt(vector<ExprPtr> a) : args(a) {}
};

class IfStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch; // can be null, or another IfStmt for else-if
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e = nullptr) : condition(c), thenBranch(t), elseBranch(e) {}
};

class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr c, StmtPtr b) : condition(c), body(b) {}
};

class DoWhileStmt : public Stmt {
public:
    StmtPtr body;
    ExprPtr condition;
    DoWhileStmt(StmtPtr b, ExprPtr c) : body(b), condition(c) {}
};

class ForStmt : public Stmt {
public:
    StmtPtr init; // can be null
    ExprPtr condition; // can be null
    ExprPtr update; // can be null
    StmtPtr body;
    ForStmt(StmtPtr i, ExprPtr c, ExprPtr u, StmtPtr b)
        : init(i), condition(c), update(u), body(b) {}
};

class ForEachStmt : public Stmt {
public:
    string type;
    string varName;
    ExprPtr iterable;
    StmtPtr body;
    ForEachStmt(string t, string v, ExprPtr it, StmtPtr b)
        : type(t), varName(v), iterable(it), body(b) {}
};

class SwitchStmt : public Stmt {
public:
    ExprPtr expr;
    struct Case {
        ExprPtr value; // null for default
        vector<StmtPtr> body;
    };
    vector<Case> cases;
    SwitchStmt(ExprPtr e, vector<Case> c) : expr(e), cases(c) {}
};

class BreakStmt : public Stmt {};
class ContinueStmt : public Stmt {};

class ReturnStmt : public Stmt {
public:
    ExprPtr value; // can be null
    ReturnStmt(ExprPtr v = nullptr) : value(v) {}
};

struct Param {
    string type;
    string name;
    bool isArray;
};

class FunctionDecl : public Stmt {
public:
    string returnType;
    string name;
    vector<Param> params;
    shared_ptr<BlockStmt> body;
    bool isStatic;
    string access; // public, private, protected
    FunctionDecl(string rt, string n, vector<Param> p, shared_ptr<BlockStmt> b,
                 bool stat = false, string acc = "public")
        : returnType(rt), name(n), params(p), body(b), isStatic(stat), access(acc) {}
};

class ClassDecl : public Stmt {
public:
    string name;
    string parent; // empty if no inheritance
    struct Field {
        string access;
        string type;
        string name;
        ExprPtr init;
        bool isArray;
        bool isStatic;
    };
    vector<Field> fields;
    vector<shared_ptr<FunctionDecl>> methods;
    vector<shared_ptr<FunctionDecl>> constructors;
    ClassDecl(string n, string p = "") : name(n), parent(p) {}
};

class TryCatchStmt : public Stmt {
public:
    shared_ptr<BlockStmt> tryBlock;
    string catchType;
    string catchVar;
    shared_ptr<BlockStmt> catchBlock; // can be null
    shared_ptr<BlockStmt> finallyBlock; // can be null
    TryCatchStmt(shared_ptr<BlockStmt> t, string ct, string cv,
                 shared_ptr<BlockStmt> cb, shared_ptr<BlockStmt> fb)
        : tryBlock(t), catchType(ct), catchVar(cv), catchBlock(cb), finallyBlock(fb) {}
};

class ThrowStmt : public Stmt {
public:
    ExprPtr expr;
    ThrowStmt(ExprPtr e) : expr(e) {}
};

class ImportStmt : public Stmt {
public:
    string path;
    ImportStmt(string p) : path(p) {}
};

// ============================================================================
// SECTION 4: PARSER
// ============================================================================

class Parser {
    vector<Token> tokens;
    int pos = 0;

    Token& cur() { return tokens[pos]; }
    Token& peek(int ahead = 1) {
        int idx = min(pos + ahead, (int)tokens.size() - 1);
        return tokens[idx];
    }
    void advance() { if (pos < (int)tokens.size() - 1) pos++; }

    void eat(TokenType t) {
        if (cur().type == t) {
            advance();
        } else {
            throw runtime_error("Line " + to_string(cur().line) + ": Expected token type " +
                                to_string(t) + " but got '" + cur().value + "'");
        }
    }

    bool check(TokenType t) { return cur().type == t; }
    bool match(TokenType t) {
        if (check(t)) { advance(); return true; }
        return false;
    }

    bool isTypeKeyword(TokenType t) {
        return t == TOK_INT || t == TOK_FLOAT || t == TOK_DOUBLE ||
               t == TOK_STRING || t == TOK_BOOL || t == TOK_CHAR ||
               t == TOK_VOID || t == TOK_AUTO;
    }

    string parseTypeName() {
        string type = cur().value;
        advance();
        // Handle array types: int[], string[], etc.
        if (check(TOK_LBRACKET) && peek().type == TOK_RBRACKET) {
            advance(); advance();
            type += "[]";
        }
        return type;
    }

    // --- Expression Parsing (precedence climbing) ---

    ExprPtr primary() {
        // Number literal
        if (check(TOK_NUMBER)) {
            int v = stoi(cur().value);
            advance();
            return make_shared<NumberLiteral>(v);
        }
        // Float literal
        if (check(TOK_FLOAT_LIT)) {
            double v = stod(cur().value);
            advance();
            return make_shared<FloatLiteral>(v);
        }
        // String literal
        if (check(TOK_STRING_LIT)) {
            string v = cur().value;
            advance();
            return make_shared<StringLiteral>(v);
        }
        // Char literal
        if (check(TOK_CHAR_LIT)) {
            string v = cur().value;
            advance();
            return make_shared<CharLiteral>(v);
        }
        // Bool literals
        if (check(TOK_TRUE)) { advance(); return make_shared<BoolLiteral>(true); }
        if (check(TOK_FALSE)) { advance(); return make_shared<BoolLiteral>(false); }
        // Null literal
        if (check(TOK_NULL_KW)) { advance(); return make_shared<NullLiteral>(); }
        // this
        if (check(TOK_THIS)) { advance(); return make_shared<ThisExpr>(); }
        // input()
        if (check(TOK_INPUT)) {
            advance();
            eat(TOK_LPAREN);
            ExprPtr prompt = nullptr;
            if (!check(TOK_RPAREN)) {
                prompt = expression();
            }
            eat(TOK_RPAREN);
            return make_shared<InputExpr>(prompt);
        }
        // new ClassName(args)
        if (check(TOK_NEW)) {
            advance();
            string className = cur().value;
            eat(TOK_IDENTIFIER);
            eat(TOK_LPAREN);
            vector<ExprPtr> args;
            if (!check(TOK_RPAREN)) {
                args.push_back(expression());
                while (match(TOK_COMMA)) args.push_back(expression());
            }
            eat(TOK_RPAREN);
            return make_shared<NewExpr>(className, args);
        }
        // Parenthesized expression or cast
        if (check(TOK_LPAREN)) {
            // Check if it's a cast: (type)expr
            if (isTypeKeyword(peek().type) && peek(2).type == TOK_RPAREN) {
                advance(); // (
                string type = cur().value;
                advance(); // type
                advance(); // )
                ExprPtr e = unaryExpr();
                return make_shared<CastExpr>(type, e);
            }
            advance(); // (
            ExprPtr e = expression();
            eat(TOK_RPAREN);
            return e;
        }
        // Array literal with braces {1, 2, 3}
        if (check(TOK_LBRACE)) {
            advance();
            vector<ExprPtr> elems;
            if (!check(TOK_RBRACE)) {
                elems.push_back(expression());
                while (match(TOK_COMMA)) elems.push_back(expression());
            }
            eat(TOK_RBRACE);
            return make_shared<ArrayLitExpr>(elems);
        }
        // Array literal with brackets [1, 2, 3]
        if (check(TOK_LBRACKET)) {
            advance();
            vector<ExprPtr> elems;
            if (!check(TOK_RBRACKET)) {
                elems.push_back(expression());
                while (match(TOK_COMMA)) elems.push_back(expression());
            }
            eat(TOK_RBRACKET);
            return make_shared<ArrayLitExpr>(elems);
        }
        // Identifier
        if (check(TOK_IDENTIFIER)) {
            string name = cur().value;
            advance();
            return make_shared<Variable>(name);
        }
        throw runtime_error("Line " + to_string(cur().line) + ": Unexpected token '" + cur().value + "'");
    }

    ExprPtr postfix() {
        ExprPtr expr = primary();
        while (true) {
            // Function call
            if (check(TOK_LPAREN)) {
                advance();
                vector<ExprPtr> args;
                if (!check(TOK_RPAREN)) {
                    args.push_back(expression());
                    while (match(TOK_COMMA)) args.push_back(expression());
                }
                eat(TOK_RPAREN);
                expr = make_shared<CallExpr>(expr, args);
            }
            // Array/index access
            else if (check(TOK_LBRACKET)) {
                advance();
                ExprPtr idx = expression();
                eat(TOK_RBRACKET);
                expr = make_shared<IndexExpr>(expr, idx);
            }
            // Member access (.)
            else if (check(TOK_DOT)) {
                advance();
                string member = cur().value;
                eat(TOK_IDENTIFIER);
                expr = make_shared<MemberExpr>(expr, member);
            }
            // Arrow access (->)
            else if (check(TOK_ARROW)) {
                advance();
                string member = cur().value;
                eat(TOK_IDENTIFIER);
                expr = make_shared<MemberExpr>(expr, member);
            }
            // Postfix ++/--
            else if (check(TOK_INCREMENT)) {
                advance();
                expr = make_shared<UnaryExpr>("++", expr, false);
            }
            else if (check(TOK_DECREMENT)) {
                advance();
                expr = make_shared<UnaryExpr>("--", expr, false);
            }
            else break;
        }
        return expr;
    }

    ExprPtr unaryExpr() {
        // Prefix !
        if (check(TOK_NOT)) {
            advance();
            return make_shared<UnaryExpr>("!", unaryExpr(), true);
        }
        // Prefix -
        if (check(TOK_MINUS)) {
            advance();
            return make_shared<UnaryExpr>("-", unaryExpr(), true);
        }
        // Prefix ++
        if (check(TOK_INCREMENT)) {
            advance();
            return make_shared<UnaryExpr>("++", unaryExpr(), true);
        }
        // Prefix --
        if (check(TOK_DECREMENT)) {
            advance();
            return make_shared<UnaryExpr>("--", unaryExpr(), true);
        }
        // Dereference *
        if (check(TOK_STAR)) {
            advance();
            return make_shared<UnaryExpr>("*", unaryExpr(), true);
        }
        // Address-of &
        if (check(TOK_AMPERSAND)) {
            advance();
            return make_shared<UnaryExpr>("&", unaryExpr(), true);
        }
        return postfix();
    }

    ExprPtr multiplication() {
        ExprPtr left = unaryExpr();
        while (check(TOK_STAR) || check(TOK_SLASH) || check(TOK_PERCENT)) {
            string op = cur().value;
            advance();
            left = make_shared<BinaryExpr>(left, op, unaryExpr());
        }
        return left;
    }

    ExprPtr addition() {
        ExprPtr left = multiplication();
        while (check(TOK_PLUS) || check(TOK_MINUS)) {
            string op = cur().value;
            advance();
            left = make_shared<BinaryExpr>(left, op, multiplication());
        }
        return left;
    }

    ExprPtr comparison() {
        ExprPtr left = addition();
        while (check(TOK_LT) || check(TOK_GT) || check(TOK_LTE) || check(TOK_GTE)) {
            string op = cur().value;
            advance();
            left = make_shared<BinaryExpr>(left, op, addition());
        }
        return left;
    }

    ExprPtr equality() {
        ExprPtr left = comparison();
        while (check(TOK_EQ) || check(TOK_NEQ)) {
            string op = cur().value;
            advance();
            left = make_shared<BinaryExpr>(left, op, comparison());
        }
        return left;
    }

    ExprPtr logicalAnd() {
        ExprPtr left = equality();
        while (check(TOK_AND)) {
            advance();
            left = make_shared<BinaryExpr>(left, "&&", equality());
        }
        return left;
    }

    ExprPtr logicalOr() {
        ExprPtr left = logicalAnd();
        while (check(TOK_OR)) {
            advance();
            left = make_shared<BinaryExpr>(left, "||", logicalAnd());
        }
        return left;
    }

    ExprPtr ternary() {
        ExprPtr expr = logicalOr();
        if (check(TOK_QUESTION)) {
            advance();
            ExprPtr thenE = expression();
            eat(TOK_COLON);
            ExprPtr elseE = ternary();
            return make_shared<TernaryExpr>(expr, thenE, elseE);
        }
        return expr;
    }

    ExprPtr assignmentExpr() {
        ExprPtr expr = ternary();
        if (check(TOK_ASSIGN) || check(TOK_PLUS_ASSIGN) || check(TOK_MINUS_ASSIGN) ||
            check(TOK_STAR_ASSIGN) || check(TOK_SLASH_ASSIGN)) {
            string op = cur().value;
            advance();
            ExprPtr val = assignmentExpr(); // right-associative
            return make_shared<AssignExpr>(expr, op, val);
        }
        return expr;
    }

    ExprPtr expression() {
        return assignmentExpr();
    }

    // --- Statement Parsing ---

    shared_ptr<BlockStmt> block() {
        eat(TOK_LBRACE);
        vector<StmtPtr> stmts;
        while (!check(TOK_RBRACE) && !check(TOK_EOF)) {
            stmts.push_back(declaration());
        }
        eat(TOK_RBRACE);
        return make_shared<BlockStmt>(stmts);
    }

    StmtPtr blockOrStmt() {
        if (check(TOK_LBRACE)) return block();
        return declaration();
    }

    StmtPtr printStatement() {
        eat(TOK_PRINT);
        eat(TOK_LPAREN);
        vector<ExprPtr> args;
        if (!check(TOK_RPAREN)) {
            args.push_back(expression());
            while (match(TOK_COMMA)) args.push_back(expression());
        }
        eat(TOK_RPAREN);
        eat(TOK_SEMICOLON);
        return make_shared<PrintStmt>(args);
    }

    StmtPtr ifStatement() {
        eat(TOK_IF);
        eat(TOK_LPAREN);
        ExprPtr cond = expression();
        eat(TOK_RPAREN);
        StmtPtr thenBranch = blockOrStmt();
        StmtPtr elseBranch = nullptr;
        if (match(TOK_ELSE)) {
            if (check(TOK_IF)) {
                elseBranch = ifStatement(); // else if chain
            } else {
                elseBranch = blockOrStmt();
            }
        }
        return make_shared<IfStmt>(cond, thenBranch, elseBranch);
    }

    StmtPtr whileStatement() {
        eat(TOK_WHILE);
        eat(TOK_LPAREN);
        ExprPtr cond = expression();
        eat(TOK_RPAREN);
        StmtPtr body = blockOrStmt();
        return make_shared<WhileStmt>(cond, body);
    }

    StmtPtr doWhileStatement() {
        eat(TOK_DO);
        StmtPtr body = blockOrStmt();
        eat(TOK_WHILE);
        eat(TOK_LPAREN);
        ExprPtr cond = expression();
        eat(TOK_RPAREN);
        eat(TOK_SEMICOLON);
        return make_shared<DoWhileStmt>(body, cond);
    }

    StmtPtr forStatement() {
        eat(TOK_FOR);
        eat(TOK_LPAREN);

        // Check for for-each: for (type var : iterable) or for (auto var : iterable)
        if ((isTypeKeyword(cur().type) || cur().type == TOK_LET) &&
            peek().type == TOK_IDENTIFIER && peek(2).type == TOK_COLON) {
            string type = cur().value;
            advance();
            string varName = cur().value;
            eat(TOK_IDENTIFIER);
            eat(TOK_COLON);
            ExprPtr iterable = expression();
            eat(TOK_RPAREN);
            StmtPtr body = blockOrStmt();
            return make_shared<ForEachStmt>(type, varName, iterable, body);
        }

        // Regular for: for (init; cond; update)
        StmtPtr init = nullptr;
        if (!check(TOK_SEMICOLON)) {
            if (isTypeKeyword(cur().type) || check(TOK_LET)) {
                init = varDeclStatement();
            } else {
                ExprPtr e = expression();
                eat(TOK_SEMICOLON);
                init = make_shared<ExprStmt>(e);
            }
        } else {
            eat(TOK_SEMICOLON);
        }

        ExprPtr cond = nullptr;
        if (!check(TOK_SEMICOLON)) cond = expression();
        eat(TOK_SEMICOLON);

        ExprPtr update = nullptr;
        if (!check(TOK_RPAREN)) update = expression();
        eat(TOK_RPAREN);

        StmtPtr body = blockOrStmt();
        return make_shared<ForStmt>(init, cond, update, body);
    }

    StmtPtr switchStatement() {
        eat(TOK_SWITCH);
        eat(TOK_LPAREN);
        ExprPtr expr = expression();
        eat(TOK_RPAREN);
        eat(TOK_LBRACE);

        vector<SwitchStmt::Case> cases;
        while (!check(TOK_RBRACE) && !check(TOK_EOF)) {
            SwitchStmt::Case c;
            if (match(TOK_CASE)) {
                c.value = expression();
                eat(TOK_COLON);
            } else if (match(TOK_DEFAULT)) {
                c.value = nullptr;
                eat(TOK_COLON);
            }
            while (!check(TOK_CASE) && !check(TOK_DEFAULT) && !check(TOK_RBRACE) && !check(TOK_EOF)) {
                c.body.push_back(declaration());
            }
            cases.push_back(c);
        }
        eat(TOK_RBRACE);
        return make_shared<SwitchStmt>(expr, cases);
    }

    StmtPtr returnStatement() {
        eat(TOK_RETURN);
        ExprPtr val = nullptr;
        if (!check(TOK_SEMICOLON)) val = expression();
        eat(TOK_SEMICOLON);
        return make_shared<ReturnStmt>(val);
    }

    StmtPtr tryStatement() {
        eat(TOK_TRY);
        auto tryBlock = block();

        string catchType = "Exception", catchVar = "e";
        shared_ptr<BlockStmt> catchBlock = nullptr;
        shared_ptr<BlockStmt> finallyBlock = nullptr;

        if (match(TOK_CATCH)) {
            eat(TOK_LPAREN);
            if (check(TOK_IDENTIFIER) || isTypeKeyword(cur().type)) {
                catchType = cur().value;
                advance();
                if (check(TOK_IDENTIFIER)) {
                    catchVar = cur().value;
                    advance();
                }
            }
            eat(TOK_RPAREN);
            catchBlock = block();
        }
        if (match(TOK_FINALLY)) {
            finallyBlock = block();
        }
        return make_shared<TryCatchStmt>(tryBlock, catchType, catchVar, catchBlock, finallyBlock);
    }

    StmtPtr throwStatement() {
        eat(TOK_THROW);
        ExprPtr expr = expression();
        eat(TOK_SEMICOLON);
        return make_shared<ThrowStmt>(expr);
    }

    StmtPtr varDeclStatement() {
        string type;
        bool isArray = false;

        if (check(TOK_LET)) {
            type = "auto";
            advance();
        } else {
            type = cur().value;
            advance();
            // Check for array type: int[] or string[]
            if (check(TOK_LBRACKET) && peek().type == TOK_RBRACKET) {
                advance(); advance();
                isArray = true;
            }
        }

        string name = cur().value;
        eat(TOK_IDENTIFIER);

        ExprPtr init = nullptr;
        if (match(TOK_ASSIGN)) {
            init = expression();
        }
        eat(TOK_SEMICOLON);
        return make_shared<VarDeclStmt>(type, name, init, isArray);
    }

    vector<Param> parseParams() {
        vector<Param> params;
        eat(TOK_LPAREN);
        if (!check(TOK_RPAREN)) {
            Param p;
            p.type = cur().value;
            advance();
            p.isArray = false;
            if (check(TOK_LBRACKET) && peek().type == TOK_RBRACKET) {
                advance(); advance();
                p.isArray = true;
            }
            p.name = cur().value;
            eat(TOK_IDENTIFIER);
            params.push_back(p);

            while (match(TOK_COMMA)) {
                Param p2;
                p2.type = cur().value;
                advance();
                p2.isArray = false;
                if (check(TOK_LBRACKET) && peek().type == TOK_RBRACKET) {
                    advance(); advance();
                    p2.isArray = true;
                }
                p2.name = cur().value;
                eat(TOK_IDENTIFIER);
                params.push_back(p2);
            }
        }
        eat(TOK_RPAREN);
        return params;
    }

    shared_ptr<FunctionDecl> functionDecl(string returnType, string name, bool isStat = false, string acc = "public") {
        auto params = parseParams();
        auto body = block();
        return make_shared<FunctionDecl>(returnType, name, params, body, isStat, acc);
    }

    StmtPtr classDeclaration() {
        eat(TOK_CLASS);
        string name = cur().value;
        eat(TOK_IDENTIFIER);

        string parent = "";
        if (match(TOK_EXTENDS) || match(TOK_COLON)) {
            if (check(TOK_PUBLIC) || check(TOK_PRIVATE) || check(TOK_PROTECTED)) advance();
            parent = cur().value;
            eat(TOK_IDENTIFIER);
        }

        auto cls = make_shared<ClassDecl>(name, parent);
        eat(TOK_LBRACE);

        string currentAccess = "public";

        while (!check(TOK_RBRACE) && !check(TOK_EOF)) {
            // Access specifiers: public: private: protected:
            if ((check(TOK_PUBLIC) || check(TOK_PRIVATE) || check(TOK_PROTECTED)) &&
                peek().type == TOK_COLON) {
                currentAccess = cur().value;
                advance(); advance();
                continue;
            }

            bool isStatic = false;
            string memberAccess = currentAccess;

            // Check for access modifier before member
            if (check(TOK_PUBLIC) || check(TOK_PRIVATE) || check(TOK_PROTECTED)) {
                if (peek().type != TOK_COLON) {
                    memberAccess = cur().value;
                    advance();
                }
            }

            if (match(TOK_STATIC)) isStatic = true;

            // Constructor: ClassName(params) { body }
            if (check(TOK_IDENTIFIER) && cur().value == name && peek().type == TOK_LPAREN) {
                string cname = cur().value;
                advance();
                auto fn = functionDecl("void", cname, isStatic, memberAccess);
                fn->returnType = ""; // mark as constructor
                cls->constructors.push_back(fn);
                match(TOK_SEMICOLON); // optional
                continue;
            }

            // Method or field
            if (isTypeKeyword(cur().type) || check(TOK_IDENTIFIER) || check(TOK_VOID)) {
                string type = cur().value;
                advance();

                bool arrType = false;
                if (check(TOK_LBRACKET) && peek().type == TOK_RBRACKET) {
                    advance(); advance();
                    arrType = true;
                }

                string mname = cur().value;
                eat(TOK_IDENTIFIER);

                if (check(TOK_LPAREN)) {
                    // Method
                    auto fn = functionDecl(type, mname, isStatic, memberAccess);
                    cls->methods.push_back(fn);
                    match(TOK_SEMICOLON); // optional
                } else {
                    // Field
                    ExprPtr init = nullptr;
                    if (match(TOK_ASSIGN)) init = expression();
                    eat(TOK_SEMICOLON);
                    cls->fields.push_back({memberAccess, type, mname, init, arrType, isStatic});
                }
                continue;
            }

            // Skip unknown
            advance();
        }
        eat(TOK_RBRACE);
        match(TOK_SEMICOLON); // optional trailing semicolon (C++ style)
        return cls;
    }

    StmtPtr importStatement() {
        string path;
        if (check(TOK_INCLUDE)) {
            path = cur().value;
            advance();
            match(TOK_SEMICOLON);
            return make_shared<ImportStmt>(path);
        }
        eat(TOK_IMPORT);
        path = "";
        while (!check(TOK_SEMICOLON) && !check(TOK_EOF)) {
            path += cur().value;
            advance();
        }
        eat(TOK_SEMICOLON);
        return make_shared<ImportStmt>(path);
    }

    StmtPtr declaration() {
        // Import/include
        if (check(TOK_INCLUDE) || check(TOK_IMPORT)) return importStatement();

        // Class declaration
        if (check(TOK_CLASS)) return classDeclaration();

        // Abstract class
        if (check(TOK_ABSTRACT) && peek().type == TOK_CLASS) {
            advance();
            return classDeclaration();
        }

        // Function declaration: type name(params) { body }
        if (isTypeKeyword(cur().type) && peek().type == TOK_IDENTIFIER &&
            peek(2).type == TOK_LPAREN) {
            string retType = cur().value;
            advance();
            string name = cur().value;
            advance();
            return functionDecl(retType, name);
        }

        // Static function: static type name(params) { body }
        if (check(TOK_STATIC) && isTypeKeyword(peek().type) &&
            peek(2).type == TOK_IDENTIFIER && peek(3).type == TOK_LPAREN) {
            advance(); // static
            string retType = cur().value;
            advance();
            string name = cur().value;
            advance();
            return functionDecl(retType, name, true);
        }

        // Function keyword: function name(params) { body }
        if (check(TOK_FUNCTION)) {
            advance();
            string name = cur().value;
            eat(TOK_IDENTIFIER);
            return functionDecl("auto", name);
        }

        return statement();
    }

    StmtPtr statement() {
        if (check(TOK_PRINT)) return printStatement();
        if (check(TOK_IF)) return ifStatement();
        if (check(TOK_WHILE)) return whileStatement();
        if (check(TOK_DO)) return doWhileStatement();
        if (check(TOK_FOR)) return forStatement();
        if (check(TOK_SWITCH)) return switchStatement();
        if (check(TOK_RETURN)) return returnStatement();
        if (check(TOK_TRY)) return tryStatement();
        if (check(TOK_THROW)) return throwStatement();
        if (check(TOK_BREAK)) { advance(); eat(TOK_SEMICOLON); return make_shared<BreakStmt>(); }
        if (check(TOK_CONTINUE)) { advance(); eat(TOK_SEMICOLON); return make_shared<ContinueStmt>(); }
        if (check(TOK_LBRACE)) return block();

        // Variable declaration: let x = ...; or type x = ...;
        if (check(TOK_LET)) return varDeclStatement();
        if (isTypeKeyword(cur().type) && peek().type == TOK_IDENTIFIER &&
            peek(2).type != TOK_LPAREN) {
            return varDeclStatement();
        }
        // Typed array declaration: int[] arr = ...;
        if (isTypeKeyword(cur().type) && peek().type == TOK_LBRACKET) {
            return varDeclStatement();
        }

        // User-defined type declaration: ClassName obj = ...;
        if (check(TOK_IDENTIFIER) && peek().type == TOK_IDENTIFIER &&
            peek(2).type != TOK_LPAREN) {
            // Could be: ClassName varName = ...;
            string type = cur().value;
            advance();
            string name = cur().value;
            eat(TOK_IDENTIFIER);
            ExprPtr init = nullptr;
            if (match(TOK_ASSIGN)) init = expression();
            eat(TOK_SEMICOLON);
            return make_shared<VarDeclStmt>(type, name, init, false);
        }

        // Expression statement
        ExprPtr e = expression();
        eat(TOK_SEMICOLON);
        return make_shared<ExprStmt>(e);
    }

public:
    Parser(vector<Token> t) : tokens(t) {}

    vector<StmtPtr> parse() {
        vector<StmtPtr> program;
        while (!check(TOK_EOF)) {
            program.push_back(declaration());
        }
        return program;
    }
};

// ============================================================================
// SECTION 5: SEMANTIC ANALYZER
// ============================================================================

class SemanticAnalyzer {
    struct Symbol {
        string name;
        string type;
        bool isFunction;
        bool isClass;
        vector<string> paramTypes;
    };

    struct Scope {
        unordered_map<string, Symbol> symbols;
        Scope* parent;
        Scope(Scope* p = nullptr) : parent(p) {}
    };

    Scope* currentScope;
    vector<string> errors;
    vector<Scope*> allScopes; // for cleanup

    void enterScope() {
        auto s = new Scope(currentScope);
        allScopes.push_back(s);
        currentScope = s;
    }

    void exitScope() {
        if (currentScope && currentScope->parent) {
            currentScope = currentScope->parent;
        }
    }

    void declare(const string& name, const string& type, bool isFunc = false, bool isCls = false) {
        if (currentScope) {
            currentScope->symbols[name] = {name, type, isFunc, isCls, {}};
        }
    }

    Symbol* lookup(const string& name) {
        Scope* s = currentScope;
        while (s) {
            auto it = s->symbols.find(name);
            if (it != s->symbols.end()) return &it->second;
            s = s->parent;
        }
        return nullptr;
    }

    string inferType(ExprPtr e) {
        if (!e) return "void";
        if (dynamic_pointer_cast<NumberLiteral>(e)) return "int";
        if (dynamic_pointer_cast<FloatLiteral>(e)) return "double";
        if (dynamic_pointer_cast<StringLiteral>(e)) return "string";
        if (dynamic_pointer_cast<BoolLiteral>(e)) return "bool";
        if (dynamic_pointer_cast<CharLiteral>(e)) return "char";
        if (dynamic_pointer_cast<NullLiteral>(e)) return "null";
        if (auto v = dynamic_pointer_cast<Variable>(e)) {
            auto sym = lookup(v->name);
            if (sym) return sym->type;
            return "auto";
        }
        if (auto b = dynamic_pointer_cast<BinaryExpr>(e)) {
            string lt = inferType(b->left);
            string rt = inferType(b->right);
            if (b->op == "==" || b->op == "!=" || b->op == "<" ||
                b->op == ">" || b->op == "<=" || b->op == ">=" ||
                b->op == "&&" || b->op == "||") return "bool";
            if (lt == "double" || rt == "double") return "double";
            if (lt == "float" || rt == "float") return "float";
            if (lt == "string" || rt == "string") return "string";
            return "int";
        }
        if (auto c = dynamic_pointer_cast<CallExpr>(e)) {
            if (auto v = dynamic_pointer_cast<Variable>(c->callee)) {
                auto sym = lookup(v->name);
                if (sym) return sym->type;
            }
            return "auto";
        }
        if (dynamic_pointer_cast<InputExpr>(e)) return "string";
        if (dynamic_pointer_cast<ArrayLitExpr>(e)) return "array";
        if (dynamic_pointer_cast<NewExpr>(e)) return "object";
        return "auto";
    }

    void analyzeExpr(ExprPtr e) {
        if (!e) return;
        if (auto v = dynamic_pointer_cast<Variable>(e)) {
            if (!lookup(v->name)) {
                errors.push_back("Line: Undeclared variable '" + v->name + "'");
            }
        }
        if (auto b = dynamic_pointer_cast<BinaryExpr>(e)) {
            analyzeExpr(b->left);
            analyzeExpr(b->right);
        }
        if (auto u = dynamic_pointer_cast<UnaryExpr>(e)) {
            analyzeExpr(u->operand);
        }
        if (auto a = dynamic_pointer_cast<AssignExpr>(e)) {
            analyzeExpr(a->target);
            analyzeExpr(a->value);
        }
        if (auto c = dynamic_pointer_cast<CallExpr>(e)) {
            analyzeExpr(c->callee);
            for (auto& arg : c->args) analyzeExpr(arg);
        }
        if (auto m = dynamic_pointer_cast<MemberExpr>(e)) {
            analyzeExpr(m->object);
        }
        if (auto i = dynamic_pointer_cast<IndexExpr>(e)) {
            analyzeExpr(i->object);
            analyzeExpr(i->index);
        }
        if (auto t = dynamic_pointer_cast<TernaryExpr>(e)) {
            analyzeExpr(t->condition);
            analyzeExpr(t->thenExpr);
            analyzeExpr(t->elseExpr);
        }
        if (auto n = dynamic_pointer_cast<NewExpr>(e)) {
            for (auto& a : n->args) analyzeExpr(a);
        }
    }

    void analyzeStmt(StmtPtr s) {
        if (!s) return;

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            analyzeExpr(vd->init);
            string t = vd->type;
            if (t == "auto" && vd->init) t = inferType(vd->init);
            declare(vd->name, t);
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            analyzeExpr(es->expr);
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            for (auto& a : ps->args) analyzeExpr(a);
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            analyzeExpr(is->condition);
            enterScope();
            analyzeStmt(is->thenBranch);
            exitScope();
            if (is->elseBranch) {
                enterScope();
                analyzeStmt(is->elseBranch);
                exitScope();
            }
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            analyzeExpr(ws->condition);
            enterScope();
            analyzeStmt(ws->body);
            exitScope();
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            enterScope();
            analyzeStmt(dw->body);
            exitScope();
            analyzeExpr(dw->condition);
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            enterScope();
            analyzeStmt(fs->init);
            analyzeExpr(fs->condition);
            analyzeExpr(fs->update);
            analyzeStmt(fs->body);
            exitScope();
        }
        else if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
            analyzeExpr(fe->iterable);
            enterScope();
            declare(fe->varName, fe->type);
            analyzeStmt(fe->body);
            exitScope();
        }
        else if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
            analyzeExpr(sw->expr);
            for (auto& c : sw->cases) {
                analyzeExpr(c.value);
                enterScope();
                for (auto& st : c.body) analyzeStmt(st);
                exitScope();
            }
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            analyzeExpr(rs->value);
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            enterScope();
            for (auto& st : bs->stmts) analyzeStmt(st);
            exitScope();
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            declare(fd->name, fd->returnType, true);
            enterScope();
            for (auto& p : fd->params) declare(p.name, p.type);
            if (fd->body) {
                for (auto& st : fd->body->stmts) analyzeStmt(st);
            }
            exitScope();
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            declare(cd->name, cd->name, false, true);
            enterScope();
            for (auto& f : cd->fields) {
                declare(f.name, f.type);
            }
            for (auto& m : cd->methods) analyzeStmt(m);
            for (auto& c : cd->constructors) analyzeStmt(c);
            exitScope();
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            enterScope();
            if (tc->tryBlock) for (auto& st : tc->tryBlock->stmts) analyzeStmt(st);
            exitScope();
            if (tc->catchBlock) {
                enterScope();
                declare(tc->catchVar, tc->catchType);
                for (auto& st : tc->catchBlock->stmts) analyzeStmt(st);
                exitScope();
            }
            if (tc->finallyBlock) {
                enterScope();
                for (auto& st : tc->finallyBlock->stmts) analyzeStmt(st);
                exitScope();
            }
        }
        else if (auto th = dynamic_pointer_cast<ThrowStmt>(s)) {
            analyzeExpr(th->expr);
        }
    }

public:
    SemanticAnalyzer() : currentScope(nullptr) {}

    ~SemanticAnalyzer() {
        for (auto s : allScopes) delete s;
    }

    vector<string> analyze(vector<StmtPtr>& program) {
        errors.clear();
        auto global = new Scope(nullptr);
        allScopes.push_back(global);
        currentScope = global;

        // Pre-declare built-in functions
        declare("print", "void", true);
        declare("input", "string", true);
        declare("println", "void", true);
        declare("len", "int", true);
        declare("size", "int", true);
        declare("push_back", "void", true);
        declare("append", "void", true);
        declare("toString", "string", true);

        for (auto& s : program) {
            analyzeStmt(s);
        }
        return errors;
    }
};

// ============================================================================
// SECTION 6: IR GENERATION
// ============================================================================

struct IR {
    string op, arg1, arg2, res;
};

class IRGenerator {
    vector<IR> code;
    int tempCount = 0;
    int labelCount = 0;

    string newTemp() { return "t" + to_string(++tempCount); }
    string newLabel() { return "L" + to_string(++labelCount); }

public:
    string genExpr(ExprPtr e) {
        if (!e) return "";
        if (auto n = dynamic_pointer_cast<NumberLiteral>(e)) return to_string(n->value);
        if (auto f = dynamic_pointer_cast<FloatLiteral>(e)) return to_string(f->value);
        if (auto s = dynamic_pointer_cast<StringLiteral>(e)) return "\"" + s->value + "\"";
        if (auto b = dynamic_pointer_cast<BoolLiteral>(e)) return b->value ? "true" : "false";
        if (auto v = dynamic_pointer_cast<Variable>(e)) return v->name;
        if (dynamic_pointer_cast<NullLiteral>(e)) return "null";
        if (dynamic_pointer_cast<ThisExpr>(e)) return "this";

        if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
            string l = genExpr(bin->left);
            string r = genExpr(bin->right);
            string t = newTemp();
            code.push_back({bin->op, l, r, t});
            return t;
        }
        if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
            string o = genExpr(un->operand);
            string t = newTemp();
            code.push_back({un->op, o, un->isPrefix ? "pre" : "post", t});
            return t;
        }
        if (auto ae = dynamic_pointer_cast<AssignExpr>(e)) {
            string v = genExpr(ae->value);
            string target = genExpr(ae->target);
            code.push_back({ae->op, v, "", target});
            return target;
        }
        if (auto ce = dynamic_pointer_cast<CallExpr>(e)) {
            vector<string> argStrs;
            for (auto& a : ce->args) {
                string ar = genExpr(a);
                code.push_back({"PARAM", ar, "", ""});
                argStrs.push_back(ar);
            }
            string callee = genExpr(ce->callee);
            string t = newTemp();
            code.push_back({"CALL", callee, to_string(ce->args.size()), t});
            return t;
        }
        if (auto ne = dynamic_pointer_cast<NewExpr>(e)) {
            for (auto& a : ne->args) {
                string ar = genExpr(a);
                code.push_back({"PARAM", ar, "", ""});
            }
            string t = newTemp();
            code.push_back({"NEW", ne->className, to_string(ne->args.size()), t});
            return t;
        }
        if (auto ie = dynamic_pointer_cast<InputExpr>(e)) {
            string t = newTemp();
            string prompt = ie->prompt ? genExpr(ie->prompt) : "";
            code.push_back({"INPUT", prompt, "", t});
            return t;
        }
        if (auto me = dynamic_pointer_cast<MemberExpr>(e)) {
            string obj = genExpr(me->object);
            string t = newTemp();
            code.push_back({"MEMBER", obj, me->member, t});
            return t;
        }
        if (auto idx = dynamic_pointer_cast<IndexExpr>(e)) {
            string obj = genExpr(idx->object);
            string i = genExpr(idx->index);
            string t = newTemp();
            code.push_back({"INDEX", obj, i, t});
            return t;
        }
        if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
            string c = genExpr(te->condition);
            string lTrue = newLabel(), lFalse = newLabel(), lEnd = newLabel();
            string t = newTemp();
            code.push_back({"IF_FALSE", c, "", lFalse});
            string tv = genExpr(te->thenExpr);
            code.push_back({"=", tv, "", t});
            code.push_back({"GOTO", "", "", lEnd});
            code.push_back({"LABEL", "", "", lFalse});
            string fv = genExpr(te->elseExpr);
            code.push_back({"=", fv, "", t});
            code.push_back({"LABEL", "", "", lEnd});
            return t;
        }
        return "";
    }

    void genStmt(StmtPtr s) {
        if (!s) return;

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            if (vd->init) {
                string v = genExpr(vd->init);
                code.push_back({"DECL", vd->type, v, vd->name});
            } else {
                code.push_back({"DECL", vd->type, "", vd->name});
            }
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            genExpr(es->expr);
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            for (auto& a : ps->args) {
                string v = genExpr(a);
                code.push_back({"PRINT", v, "", ""});
            }
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            string c = genExpr(is->condition);
            string lElse = newLabel();
            string lEnd = newLabel();
            code.push_back({"IF_FALSE", c, "", is->elseBranch ? lElse : lEnd});
            genStmt(is->thenBranch);
            if (is->elseBranch) {
                code.push_back({"GOTO", "", "", lEnd});
                code.push_back({"LABEL", "", "", lElse});
                genStmt(is->elseBranch);
            }
            code.push_back({"LABEL", "", "", lEnd});
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            string lStart = newLabel();
            string lEnd = newLabel();
            code.push_back({"LABEL", "", "", lStart});
            string c = genExpr(ws->condition);
            code.push_back({"IF_FALSE", c, "", lEnd});
            genStmt(ws->body);
            code.push_back({"GOTO", "", "", lStart});
            code.push_back({"LABEL", "", "", lEnd});
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            string lStart = newLabel();
            code.push_back({"LABEL", "", "", lStart});
            genStmt(dw->body);
            string c = genExpr(dw->condition);
            code.push_back({"IF_TRUE", c, "", lStart});
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            genStmt(fs->init);
            string lStart = newLabel();
            string lEnd = newLabel();
            code.push_back({"LABEL", "", "", lStart});
            if (fs->condition) {
                string c = genExpr(fs->condition);
                code.push_back({"IF_FALSE", c, "", lEnd});
            }
            genStmt(fs->body);
            if (fs->update) genExpr(fs->update);
            code.push_back({"GOTO", "", "", lStart});
            code.push_back({"LABEL", "", "", lEnd});
        }
        else if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
            string it = genExpr(fe->iterable);
            string lStart = newLabel();
            string lEnd = newLabel();
            code.push_back({"FOR_EACH", fe->varName, it, lStart});
            genStmt(fe->body);
            code.push_back({"GOTO", "", "", lStart});
            code.push_back({"LABEL", "", "", lEnd});
        }
        else if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
            string v = genExpr(sw->expr);
            code.push_back({"SWITCH", v, "", ""});
            for (auto& c : sw->cases) {
                if (c.value) {
                    string cv = genExpr(c.value);
                    code.push_back({"CASE", cv, "", ""});
                } else {
                    code.push_back({"DEFAULT", "", "", ""});
                }
                for (auto& st : c.body) genStmt(st);
            }
            code.push_back({"SWITCH_END", "", "", ""});
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            string v = rs->value ? genExpr(rs->value) : "";
            code.push_back({"RETURN", v, "", ""});
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            for (auto& st : bs->stmts) genStmt(st);
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            string paramStr = "";
            for (size_t i = 0; i < fd->params.size(); i++) {
                if (i > 0) paramStr += ",";
                paramStr += fd->params[i].type + " " + fd->params[i].name;
            }
            code.push_back({"FUNC_BEGIN", fd->returnType, paramStr, fd->name});
            if (fd->body) for (auto& st : fd->body->stmts) genStmt(st);
            code.push_back({"FUNC_END", "", "", fd->name});
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            code.push_back({"CLASS_BEGIN", cd->parent, "", cd->name});
            for (auto& f : cd->fields) {
                code.push_back({"FIELD", f.type, f.access, f.name});
            }
            for (auto& c : cd->constructors) genStmt(c);
            for (auto& m : cd->methods) genStmt(m);
            code.push_back({"CLASS_END", "", "", cd->name});
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            code.push_back({"TRY_BEGIN", "", "", ""});
            if (tc->tryBlock) for (auto& st : tc->tryBlock->stmts) genStmt(st);
            code.push_back({"TRY_END", "", "", ""});
            if (tc->catchBlock) {
                code.push_back({"CATCH_BEGIN", tc->catchType, "", tc->catchVar});
                for (auto& st : tc->catchBlock->stmts) genStmt(st);
                code.push_back({"CATCH_END", "", "", ""});
            }
            if (tc->finallyBlock) {
                code.push_back({"FINALLY_BEGIN", "", "", ""});
                for (auto& st : tc->finallyBlock->stmts) genStmt(st);
                code.push_back({"FINALLY_END", "", "", ""});
            }
        }
        else if (auto th = dynamic_pointer_cast<ThrowStmt>(s)) {
            string v = genExpr(th->expr);
            code.push_back({"THROW", v, "", ""});
        }
        else if (dynamic_pointer_cast<BreakStmt>(s)) {
            code.push_back({"BREAK", "", "", ""});
        }
        else if (dynamic_pointer_cast<ContinueStmt>(s)) {
            code.push_back({"CONTINUE", "", "", ""});
        }
    }

    vector<IR> generate(vector<StmtPtr>& program) {
        code.clear();
        for (auto& s : program) genStmt(s);
        return code;
    }
};

// ============================================================================
// SECTION 7: OPTIMIZER
// ============================================================================

class Optimizer {
public:
    // Constant folding
    static ExprPtr foldConstants(ExprPtr e) {
        if (!e) return e;

        if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
            bin->left = foldConstants(bin->left);
            bin->right = foldConstants(bin->right);

            auto ln = dynamic_pointer_cast<NumberLiteral>(bin->left);
            auto rn = dynamic_pointer_cast<NumberLiteral>(bin->right);
            if (ln && rn) {
                int result = 0;
                if (bin->op == "+") result = ln->value + rn->value;
                else if (bin->op == "-") result = ln->value - rn->value;
                else if (bin->op == "*") result = ln->value * rn->value;
                else if (bin->op == "/" && rn->value != 0) result = ln->value / rn->value;
                else if (bin->op == "%") result = ln->value % rn->value;
                else return e;
                return make_shared<NumberLiteral>(result);
            }

            // Algebraic simplification
            if (rn) {
                if (bin->op == "+" && rn->value == 0) return bin->left;  // x + 0 = x
                if (bin->op == "-" && rn->value == 0) return bin->left;  // x - 0 = x
                if (bin->op == "*" && rn->value == 1) return bin->left;  // x * 1 = x
                if (bin->op == "*" && rn->value == 0) return make_shared<NumberLiteral>(0);
                if (bin->op == "/" && rn->value == 1) return bin->left;  // x / 1 = x
            }
            if (ln) {
                if (bin->op == "+" && ln->value == 0) return bin->right; // 0 + x = x
                if (bin->op == "*" && ln->value == 1) return bin->right; // 1 * x = x
                if (bin->op == "*" && ln->value == 0) return make_shared<NumberLiteral>(0);
            }
        }

        if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
            un->operand = foldConstants(un->operand);
            if (un->op == "-") {
                if (auto n = dynamic_pointer_cast<NumberLiteral>(un->operand))
                    return make_shared<NumberLiteral>(-n->value);
            }
            if (un->op == "!") {
                if (auto b = dynamic_pointer_cast<BoolLiteral>(un->operand))
                    return make_shared<BoolLiteral>(!b->value);
            }
        }

        if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
            te->condition = foldConstants(te->condition);
            te->thenExpr = foldConstants(te->thenExpr);
            te->elseExpr = foldConstants(te->elseExpr);
            // If condition is a bool literal, simplify
            if (auto b = dynamic_pointer_cast<BoolLiteral>(te->condition)) {
                return b->value ? te->thenExpr : te->elseExpr;
            }
        }

        return e;
    }

    static void optimizeStmt(StmtPtr s) {
        if (!s) return;

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            vd->init = foldConstants(vd->init);
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            es->expr = foldConstants(es->expr);
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            for (auto& a : ps->args) a = foldConstants(a);
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            is->condition = foldConstants(is->condition);
            optimizeStmt(is->thenBranch);
            optimizeStmt(is->elseBranch);
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            ws->condition = foldConstants(ws->condition);
            optimizeStmt(ws->body);
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            dw->condition = foldConstants(dw->condition);
            optimizeStmt(dw->body);
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            optimizeStmt(fs->init);
            fs->condition = foldConstants(fs->condition);
            fs->update = foldConstants(fs->update);
            optimizeStmt(fs->body);
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            rs->value = foldConstants(rs->value);
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            for (auto& st : bs->stmts) optimizeStmt(st);
            // Dead code elimination: remove statements after return
            auto& stmts = bs->stmts;
            for (size_t i = 0; i < stmts.size(); i++) {
                if (dynamic_pointer_cast<ReturnStmt>(stmts[i]) ||
                    dynamic_pointer_cast<BreakStmt>(stmts[i]) ||
                    dynamic_pointer_cast<ContinueStmt>(stmts[i])) {
                    stmts.resize(i + 1);
                    break;
                }
            }
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            if (fd->body) optimizeStmt(fd->body);
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            for (auto& m : cd->methods) optimizeStmt(m);
            for (auto& c : cd->constructors) optimizeStmt(c);
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            if (tc->tryBlock) optimizeStmt(tc->tryBlock);
            if (tc->catchBlock) optimizeStmt(tc->catchBlock);
            if (tc->finallyBlock) optimizeStmt(tc->finallyBlock);
        }
    }

    static void optimize(vector<StmtPtr>& program) {
        for (auto& s : program) optimizeStmt(s);
    }

    // IR-level optimizations
    static vector<IR> optimizeIR(vector<IR> code) {
        // Copy propagation on IR
        unordered_map<string, string> copies;
        vector<IR> optimized;
        for (auto& ir : code) {
            IR inst = ir;
            // Substitute known copies
            if (copies.count(inst.arg1)) inst.arg1 = copies[inst.arg1];
            if (copies.count(inst.arg2)) inst.arg2 = copies[inst.arg2];

            // Track simple copies: x = y (no operation)
            if (inst.op == "=" && !inst.arg1.empty() && inst.res != inst.arg1) {
                copies[inst.res] = inst.arg1;
            }
            optimized.push_back(inst);
        }
        return optimized;
    }
};

// ============================================================================
// SECTION 8: CODE GENERATORS (AST-walking)
// ============================================================================

// --- Type inference utility ---
// Infers concrete type from an initializer expression so that
// `let x = 10;` produces `int x = 10;` instead of `auto`/`var`.
string inferTypeFromExpr(ExprPtr e) {
    if (!e) return "int"; // default
    if (dynamic_pointer_cast<NumberLiteral>(e)) return "int";
    if (dynamic_pointer_cast<FloatLiteral>(e)) return "double";
    if (dynamic_pointer_cast<StringLiteral>(e)) return "string";
    if (dynamic_pointer_cast<BoolLiteral>(e)) return "bool";
    if (dynamic_pointer_cast<CharLiteral>(e)) return "char";
    if (dynamic_pointer_cast<NullLiteral>(e)) return "int";
    if (dynamic_pointer_cast<InputExpr>(e)) return "string";
    if (dynamic_pointer_cast<ArrayLitExpr>(e)) return "int"; // arrays handled separately
    if (dynamic_pointer_cast<NewExpr>(e)) {
        auto ne = dynamic_pointer_cast<NewExpr>(e);
        return ne->className;
    }
    if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
        // Comparison/logical operators produce bool
        if (bin->op == "==" || bin->op == "!=" || bin->op == "<" ||
            bin->op == ">" || bin->op == "<=" || bin->op == ">=" ||
            bin->op == "&&" || bin->op == "||") return "bool";
        // Arithmetic: check if either side is float/double/string
        string lt = inferTypeFromExpr(bin->left);
        string rt = inferTypeFromExpr(bin->right);
        if (lt == "double" || rt == "double") return "double";
        if (lt == "float" || rt == "float") return "float";
        if (lt == "string" || rt == "string") return "string";
        return "int";
    }
    if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
        if (un->op == "!") return "bool";
        return inferTypeFromExpr(un->operand);
    }
    if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
        return inferTypeFromExpr(te->thenExpr);
    }
    return "int"; // safe default
}

// Resolve 'auto' to a concrete type using the initializer
string resolveAutoType(const string& type, ExprPtr init) {
    if (type == "auto" && init) return inferTypeFromExpr(init);
    return type;
}

// --- Type mapping utilities ---
string mapTypeToCpp(const string& type) {
    if (type == "string") return "string";
    if (type == "bool") return "bool";
    if (type == "auto") return "int"; // fallback if unresolved
    return type; // int, float, double, char, void pass through
}

string mapTypeToJava(const string& type) {
    if (type == "string") return "String";
    if (type == "bool") return "boolean";
    if (type == "auto") return "int"; // fallback if unresolved
    if (type == "double") return "double";
    if (type == "float") return "float";
    return type;
}

string mapBoxedTypeJava(const string& type) {
    if (type == "int") return "Integer";
    if (type == "double") return "Double";
    if (type == "float") return "Float";
    if (type == "bool" || type == "boolean") return "Boolean";
    if (type == "char") return "Character";
    if (type == "string") return "String";
    return type;
}

// ==========================================================================
// PYTHON GENERATOR
// ==========================================================================

class PythonGenerator {
    int indentLevel = 0;
    string indent() { return string(indentLevel * 4, ' '); }
    bool usesInput = false;

public:
    string genExpr(ExprPtr e) {
        if (!e) return "";
        if (auto n = dynamic_pointer_cast<NumberLiteral>(e)) return to_string(n->value);
        if (auto f = dynamic_pointer_cast<FloatLiteral>(e)) {
            ostringstream oss; oss << f->value; return oss.str();
        }
        if (auto s = dynamic_pointer_cast<StringLiteral>(e)) return "\"" + s->value + "\"";
        if (auto b = dynamic_pointer_cast<BoolLiteral>(e)) return b->value ? "True" : "False";
        if (auto c = dynamic_pointer_cast<CharLiteral>(e)) return "'" + c->value + "'";
        if (dynamic_pointer_cast<NullLiteral>(e)) return "None";
        if (auto v = dynamic_pointer_cast<Variable>(e)) return v->name;
        if (dynamic_pointer_cast<ThisExpr>(e)) return "self";

        if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
            string op = bin->op;
            if (op == "&&") op = "and";
            else if (op == "||") op = "or";
            return genExpr(bin->left) + " " + op + " " + genExpr(bin->right);
        }
        if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
            string op = un->op;
            if (op == "!") op = "not ";
            if (un->isPrefix) {
                if (op == "++" || op == "--") {
                    // Python doesn't have ++/--, emit as += 1 or -= 1 is a statement
                    // In expression context, just return the value
                    return genExpr(un->operand);
                }
                return op + genExpr(un->operand);
            } else {
                return genExpr(un->operand);
            }
        }
        if (auto ae = dynamic_pointer_cast<AssignExpr>(e)) {
            return genExpr(ae->target) + " " + ae->op + " " + genExpr(ae->value);
        }
        if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
            return genExpr(te->thenExpr) + " if " + genExpr(te->condition) + " else " + genExpr(te->elseExpr);
        }
        if (auto ce = dynamic_pointer_cast<CallExpr>(e)) {
            string callee = genExpr(ce->callee);
            string args = "";
            for (size_t i = 0; i < ce->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ce->args[i]);
            }
            return callee + "(" + args + ")";
        }
        if (auto ie = dynamic_pointer_cast<IndexExpr>(e)) {
            return genExpr(ie->object) + "[" + genExpr(ie->index) + "]";
        }
        if (auto me = dynamic_pointer_cast<MemberExpr>(e)) {
            return genExpr(me->object) + "." + me->member;
        }
        if (auto ne = dynamic_pointer_cast<NewExpr>(e)) {
            string args = "";
            for (size_t i = 0; i < ne->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ne->args[i]);
            }
            return ne->className + "(" + args + ")";
        }
        if (auto al = dynamic_pointer_cast<ArrayLitExpr>(e)) {
            string elems = "";
            for (size_t i = 0; i < al->elements.size(); i++) {
                if (i > 0) elems += ", ";
                elems += genExpr(al->elements[i]);
            }
            return "[" + elems + "]";
        }
        if (auto inp = dynamic_pointer_cast<InputExpr>(e)) {
            usesInput = true;
            if (inp->prompt) return "input(" + genExpr(inp->prompt) + ")";
            return "input()";
        }
        if (auto cast = dynamic_pointer_cast<CastExpr>(e)) {
            string t = cast->targetType;
            if (t == "int") return "int(" + genExpr(cast->expr) + ")";
            if (t == "float" || t == "double") return "float(" + genExpr(cast->expr) + ")";
            if (t == "string") return "str(" + genExpr(cast->expr) + ")";
            if (t == "bool") return "bool(" + genExpr(cast->expr) + ")";
            return genExpr(cast->expr);
        }
        return "";
    }

    string genStmt(StmtPtr s) {
        if (!s) return "";
        string out = "";

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            out += indent() + vd->name;
            if (vd->init) out += " = " + genExpr(vd->init);
            else out += " = None";
            out += "\n";
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            string expr = genExpr(es->expr);
            // Handle ++/-- as statements
            if (auto ae = dynamic_pointer_cast<AssignExpr>(es->expr)) {
                out += indent() + expr + "\n";
            }
            else if (auto un = dynamic_pointer_cast<UnaryExpr>(es->expr)) {
                if (un->op == "++" || un->op == "--") {
                    string v = genExpr(un->operand);
                    out += indent() + v + (un->op == "++" ? " += 1" : " -= 1") + "\n";
                } else {
                    out += indent() + expr + "\n";
                }
            }
            else {
                out += indent() + expr + "\n";
            }
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            out += indent() + "print(";
            for (size_t i = 0; i < ps->args.size(); i++) {
                if (i > 0) out += ", ";
                out += genExpr(ps->args[i]);
            }
            out += ")\n";
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            out += indent() + "if " + genExpr(is->condition) + ":\n";
            indentLevel++;
            out += genBlock(is->thenBranch);
            indentLevel--;
            if (is->elseBranch) {
                // Check if else branch is another if (elif)
                if (auto elseIf = dynamic_pointer_cast<IfStmt>(is->elseBranch)) {
                    out += indent() + "elif " + genExpr(elseIf->condition) + ":\n";
                    indentLevel++;
                    out += genBlock(elseIf->thenBranch);
                    indentLevel--;
                    // Continue elif chain
                    auto eb = elseIf->elseBranch;
                    while (eb) {
                        if (auto ei = dynamic_pointer_cast<IfStmt>(eb)) {
                            out += indent() + "elif " + genExpr(ei->condition) + ":\n";
                            indentLevel++;
                            out += genBlock(ei->thenBranch);
                            indentLevel--;
                            eb = ei->elseBranch;
                        } else {
                            out += indent() + "else:\n";
                            indentLevel++;
                            out += genBlock(eb);
                            indentLevel--;
                            break;
                        }
                    }
                } else {
                    out += indent() + "else:\n";
                    indentLevel++;
                    out += genBlock(is->elseBranch);
                    indentLevel--;
                }
            }
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            out += indent() + "while " + genExpr(ws->condition) + ":\n";
            indentLevel++;
            out += genBlock(ws->body);
            indentLevel--;
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            // Python has no do-while, simulate with while True + break
            out += indent() + "while True:\n";
            indentLevel++;
            out += genBlock(dw->body);
            out += indent() + "if not (" + genExpr(dw->condition) + "):\n";
            indentLevel++;
            out += indent() + "break\n";
            indentLevel--;
            indentLevel--;
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            // Try to detect simple for(int i = start; i < end; i++) → range()
            if (tryGenForRange(fs, out)) {
                // Generated as for i in range(...)
            } else {
                // General case: convert to while loop
                if (fs->init) out += genStmt(fs->init);
                out += indent() + "while ";
                out += (fs->condition ? genExpr(fs->condition) : "True");
                out += ":\n";
                indentLevel++;
                out += genBlock(fs->body);
                if (fs->update) {
                    // Emit update as statement
                    auto tmpExpr = make_shared<ExprStmt>(fs->update);
                    out += genStmt(tmpExpr);
                }
                indentLevel--;
            }
        }
        else if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
            out += indent() + "for " + fe->varName + " in " + genExpr(fe->iterable) + ":\n";
            indentLevel++;
            out += genBlock(fe->body);
            indentLevel--;
        }
        else if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
            // Python 3.10+ has match/case, but for compatibility use if/elif/else
            bool first = true;
            string switchExpr = genExpr(sw->expr);
            for (auto& c : sw->cases) {
                if (c.value) {
                    out += indent() + (first ? "if " : "elif ");
                    out += switchExpr + " == " + genExpr(c.value) + ":\n";
                    first = false;
                } else {
                    out += indent() + "else:\n";
                }
                indentLevel++;
                for (auto& st : c.body) {
                    if (!dynamic_pointer_cast<BreakStmt>(st)) {
                        out += genStmt(st);
                    }
                }
                indentLevel--;
            }
        }
        else if (dynamic_pointer_cast<BreakStmt>(s)) {
            out += indent() + "break\n";
        }
        else if (dynamic_pointer_cast<ContinueStmt>(s)) {
            out += indent() + "continue\n";
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            out += indent() + "return";
            if (rs->value) out += " " + genExpr(rs->value);
            out += "\n";
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            out += indent() + "def " + fd->name + "(";
            for (size_t i = 0; i < fd->params.size(); i++) {
                if (i > 0) out += ", ";
                out += fd->params[i].name;
            }
            out += "):\n";
            indentLevel++;
            if (fd->body && !fd->body->stmts.empty()) {
                for (auto& st : fd->body->stmts) out += genStmt(st);
            } else {
                out += indent() + "pass\n";
            }
            indentLevel--;
            out += "\n";
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            out += indent() + "class " + cd->name;
            if (!cd->parent.empty()) out += "(" + cd->parent + ")";
            out += ":\n";
            indentLevel++;

            bool hasContent = false;

            // Constructor(s)
            for (auto& ctor : cd->constructors) {
                hasContent = true;
                out += indent() + "def __init__(self";
                for (auto& p : ctor->params) {
                    out += ", " + p.name;
                }
                out += "):\n";
                indentLevel++;
                if (ctor->body && !ctor->body->stmts.empty()) {
                    for (auto& st : ctor->body->stmts) out += genStmt(st);
                } else {
                    out += indent() + "pass\n";
                }
                indentLevel--;
                out += "\n";
            }

            // If no constructors but has fields, generate __init__
            if (cd->constructors.empty() && !cd->fields.empty()) {
                hasContent = true;
                out += indent() + "def __init__(self):\n";
                indentLevel++;
                for (auto& f : cd->fields) {
                    out += indent() + "self." + f.name;
                    if (f.init) out += " = " + genExpr(f.init);
                    else out += " = None";
                    out += "\n";
                }
                indentLevel--;
                out += "\n";
            }

            // Methods
            for (auto& m : cd->methods) {
                hasContent = true;
                out += indent() + "def " + m->name + "(self";
                for (auto& p : m->params) {
                    out += ", " + p.name;
                }
                out += "):\n";
                indentLevel++;
                if (m->body && !m->body->stmts.empty()) {
                    for (auto& st : m->body->stmts) out += genStmt(st);
                } else {
                    out += indent() + "pass\n";
                }
                indentLevel--;
                out += "\n";
            }

            if (!hasContent) out += indent() + "pass\n";
            indentLevel--;
            out += "\n";
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            out += indent() + "try:\n";
            indentLevel++;
            if (tc->tryBlock) {
                for (auto& st : tc->tryBlock->stmts) out += genStmt(st);
            }
            if (!tc->tryBlock || tc->tryBlock->stmts.empty()) out += indent() + "pass\n";
            indentLevel--;

            if (tc->catchBlock) {
                out += indent() + "except " + tc->catchType + " as " + tc->catchVar + ":\n";
                indentLevel++;
                for (auto& st : tc->catchBlock->stmts) out += genStmt(st);
                if (tc->catchBlock->stmts.empty()) out += indent() + "pass\n";
                indentLevel--;
            }

            if (tc->finallyBlock) {
                out += indent() + "finally:\n";
                indentLevel++;
                for (auto& st : tc->finallyBlock->stmts) out += genStmt(st);
                if (tc->finallyBlock->stmts.empty()) out += indent() + "pass\n";
                indentLevel--;
            }
        }
        else if (auto th = dynamic_pointer_cast<ThrowStmt>(s)) {
            out += indent() + "raise " + genExpr(th->expr) + "\n";
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            for (auto& st : bs->stmts) out += genStmt(st);
        }
        else if (auto imp = dynamic_pointer_cast<ImportStmt>(s)) {
            // Map C++ includes to Python imports (best effort)
            string path = imp->path;
            if (path == "iostream" || path == "cstdio") { /* no-op in Python */ }
            else if (path == "vector" || path == "list" || path == "map" ||
                     path == "set" || path == "queue" || path == "deque" ||
                     path == "stack") {
                out += indent() + "from collections import deque\n";
            }
            else if (path.find("java.util") != string::npos) {
                /* no-op */
            }
            else {
                out += indent() + "# import " + path + "\n";
            }
        }

        return out;
    }

    string genBlock(StmtPtr s) {
        if (!s) return indent() + "pass\n";
        if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            if (bs->stmts.empty()) return indent() + "pass\n";
            string out = "";
            for (auto& st : bs->stmts) out += genStmt(st);
            return out;
        }
        return genStmt(s);
    }

    bool tryGenForRange(shared_ptr<ForStmt> fs, string& out) {
        // Check if init is VarDecl: int i = start
        auto initDecl = dynamic_pointer_cast<VarDeclStmt>(fs->init);
        if (!initDecl || !initDecl->init) return false;

        string varName = initDecl->name;

        // Check condition: i < end or i <= end
        auto cond = dynamic_pointer_cast<BinaryExpr>(fs->condition);
        if (!cond) return false;
        auto condVar = dynamic_pointer_cast<Variable>(cond->left);
        if (!condVar || condVar->name != varName) return false;
        if (cond->op != "<" && cond->op != "<=") return false;

        // Check update: i++ or i += 1 or i = i + 1
        bool simpleIncrement = false;
        ExprPtr stepExpr = nullptr;

        if (auto upUn = dynamic_pointer_cast<UnaryExpr>(fs->update)) {
            if (upUn->op == "++" || upUn->op == "--") {
                auto upVar = dynamic_pointer_cast<Variable>(upUn->operand);
                if (upVar && upVar->name == varName) {
                    simpleIncrement = (upUn->op == "++");
                    if (!simpleIncrement) return false; // decrement range is complex
                }
            }
        }
        if (auto upAs = dynamic_pointer_cast<AssignExpr>(fs->update)) {
            auto upVar = dynamic_pointer_cast<Variable>(upAs->target);
            if (upVar && upVar->name == varName) {
                if (upAs->op == "+=") {
                    stepExpr = upAs->value;
                    auto sn = dynamic_pointer_cast<NumberLiteral>(stepExpr);
                    if (sn && sn->value == 1) simpleIncrement = true;
                } else if (upAs->op == "=") {
                    // i = i + step
                    auto addExpr = dynamic_pointer_cast<BinaryExpr>(upAs->value);
                    if (addExpr && addExpr->op == "+") {
                        auto addVar = dynamic_pointer_cast<Variable>(addExpr->left);
                        if (addVar && addVar->name == varName) {
                            stepExpr = addExpr->right;
                            auto sn = dynamic_pointer_cast<NumberLiteral>(stepExpr);
                            if (sn && sn->value == 1) simpleIncrement = true;
                        }
                    }
                }
            }
        }

        if (!simpleIncrement && !stepExpr) return false;

        string startStr = genExpr(initDecl->init);
        string endStr = genExpr(cond->right);
        if (cond->op == "<=") {
            // i <= end → range(start, end + 1)
            endStr = endStr + " + 1";
        }

        auto startNum = dynamic_pointer_cast<NumberLiteral>(initDecl->init);

        out += indent() + "for " + varName + " in range(";
        if (simpleIncrement) {
            if (startNum && startNum->value == 0) {
                out += endStr;
            } else {
                out += startStr + ", " + endStr;
            }
        } else {
            out += startStr + ", " + endStr + ", " + genExpr(stepExpr);
        }
        out += "):\n";
        indentLevel++;
        out += genBlock(fs->body);
        indentLevel--;
        return true;
    }

    string generate(vector<StmtPtr>& program) {
        indentLevel = 0;
        usesInput = false;
        string out = "";

        // Separate top-level declarations from main statements
        vector<StmtPtr> imports, classes, functions, mainStmts;
        for (auto& s : program) {
            if (dynamic_pointer_cast<ImportStmt>(s)) imports.push_back(s);
            else if (dynamic_pointer_cast<ClassDecl>(s)) classes.push_back(s);
            else if (dynamic_pointer_cast<FunctionDecl>(s)) functions.push_back(s);
            else mainStmts.push_back(s);
        }

        // Imports
        for (auto& s : imports) out += genStmt(s);
        if (!imports.empty()) out += "\n";

        // Classes
        for (auto& s : classes) out += genStmt(s);

        // Functions
        for (auto& s : functions) out += genStmt(s);

        // Main statements
        for (auto& s : mainStmts) out += genStmt(s);

        return out;
    }
};

// ==========================================================================
// C++ GENERATOR
// ==========================================================================

class CppGenerator {
    int indentLevel = 0;
    string indent() { return string(indentLevel * 4, ' '); }
    bool usesInput = false;
    bool usesVector = false;
    bool usesMap = false;
    bool usesString = true;
    unordered_set<string> declaredVars;
    bool inClass = false;

public:
    string genExpr(ExprPtr e) {
        if (!e) return "";
        if (auto n = dynamic_pointer_cast<NumberLiteral>(e)) return to_string(n->value);
        if (auto f = dynamic_pointer_cast<FloatLiteral>(e)) {
            ostringstream oss; oss << f->value; return oss.str();
        }
        if (auto s = dynamic_pointer_cast<StringLiteral>(e)) return "\"" + s->value + "\"";
        if (auto b = dynamic_pointer_cast<BoolLiteral>(e)) return b->value ? "true" : "false";
        if (auto c = dynamic_pointer_cast<CharLiteral>(e)) return "'" + c->value + "'";
        if (dynamic_pointer_cast<NullLiteral>(e)) return "nullptr";
        if (auto v = dynamic_pointer_cast<Variable>(e)) return v->name;
        if (dynamic_pointer_cast<ThisExpr>(e)) return "this";

        if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
            return genExpr(bin->left) + " " + bin->op + " " + genExpr(bin->right);
        }
        if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
            if (un->isPrefix) return un->op + genExpr(un->operand);
            else return genExpr(un->operand) + un->op;
        }
        if (auto ae = dynamic_pointer_cast<AssignExpr>(e)) {
            return genExpr(ae->target) + " " + ae->op + " " + genExpr(ae->value);
        }
        if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
            return genExpr(te->condition) + " ? " + genExpr(te->thenExpr) + " : " + genExpr(te->elseExpr);
        }
        if (auto ce = dynamic_pointer_cast<CallExpr>(e)) {
            string callee = genExpr(ce->callee);
            string args = "";
            for (size_t i = 0; i < ce->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ce->args[i]);
            }
            return callee + "(" + args + ")";
        }
        if (auto ie = dynamic_pointer_cast<IndexExpr>(e)) {
            return genExpr(ie->object) + "[" + genExpr(ie->index) + "]";
        }
        if (auto me = dynamic_pointer_cast<MemberExpr>(e)) {
            return genExpr(me->object) + "." + me->member;
        }
        if (auto ne = dynamic_pointer_cast<NewExpr>(e)) {
            string args = "";
            for (size_t i = 0; i < ne->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ne->args[i]);
            }
            return "new " + ne->className + "(" + args + ")";
        }
        if (auto al = dynamic_pointer_cast<ArrayLitExpr>(e)) {
            string elems = "";
            for (size_t i = 0; i < al->elements.size(); i++) {
                if (i > 0) elems += ", ";
                elems += genExpr(al->elements[i]);
            }
            return "{" + elems + "}";
        }
        if (auto inp = dynamic_pointer_cast<InputExpr>(e)) {
            usesInput = true;
            return "__input__"; // placeholder, handled in statement context
        }
        if (auto cast = dynamic_pointer_cast<CastExpr>(e)) {
            return "(" + mapTypeToCpp(cast->targetType) + ")" + genExpr(cast->expr);
        }
        return "";
    }

    string genStmt(StmtPtr s) {
        if (!s) return "";
        string out = "";

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            string type = mapTypeToCpp(resolveAutoType(vd->type, vd->init));
            if (vd->isArray) {
                usesVector = true;
                type = "vector<" + type + ">";
            }
            out += indent();
            // Check if init is input
            if (vd->init && dynamic_pointer_cast<InputExpr>(vd->init)) {
                usesInput = true;
                out += type + " " + vd->name + ";\n";
                out += indent() + "cin >> " + vd->name + ";\n";
            } else {
                out += type + " " + vd->name;
                if (vd->init) out += " = " + genExpr(vd->init);
                out += ";\n";
            }
            declaredVars.insert(vd->name);
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            // Check if it's an assignment to input
            if (auto ae = dynamic_pointer_cast<AssignExpr>(es->expr)) {
                if (dynamic_pointer_cast<InputExpr>(ae->value)) {
                    usesInput = true;
                    out += indent() + "cin >> " + genExpr(ae->target) + ";\n";
                    return out;
                }
            }
            out += indent() + genExpr(es->expr) + ";\n";
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            out += indent() + "cout";
            for (auto& a : ps->args) {
                out += " << " + genExpr(a);
            }
            out += " << endl;\n";
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            out += indent() + "if (" + genExpr(is->condition) + ") {\n";
            indentLevel++;
            out += genBlock(is->thenBranch);
            indentLevel--;
            out += indent() + "}";
            if (is->elseBranch) {
                if (auto elseIf = dynamic_pointer_cast<IfStmt>(is->elseBranch)) {
                    out += " else if (" + genExpr(elseIf->condition) + ") {\n";
                    indentLevel++;
                    out += genBlock(elseIf->thenBranch);
                    indentLevel--;
                    out += indent() + "}";
                    auto eb = elseIf->elseBranch;
                    while (eb) {
                        if (auto ei = dynamic_pointer_cast<IfStmt>(eb)) {
                            out += " else if (" + genExpr(ei->condition) + ") {\n";
                            indentLevel++;
                            out += genBlock(ei->thenBranch);
                            indentLevel--;
                            out += indent() + "}";
                            eb = ei->elseBranch;
                        } else {
                            out += " else {\n";
                            indentLevel++;
                            out += genBlock(eb);
                            indentLevel--;
                            out += indent() + "}";
                            break;
                        }
                    }
                } else {
                    out += " else {\n";
                    indentLevel++;
                    out += genBlock(is->elseBranch);
                    indentLevel--;
                    out += indent() + "}";
                }
            }
            out += "\n";
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            out += indent() + "while (" + genExpr(ws->condition) + ") {\n";
            indentLevel++;
            out += genBlock(ws->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            out += indent() + "do {\n";
            indentLevel++;
            out += genBlock(dw->body);
            indentLevel--;
            out += indent() + "} while (" + genExpr(dw->condition) + ");\n";
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            out += indent() + "for (";
            if (fs->init) {
                if (auto vd = dynamic_pointer_cast<VarDeclStmt>(fs->init)) {
                    string type = mapTypeToCpp(resolveAutoType(vd->type, vd->init));
                    out += type + " " + vd->name;
                    if (vd->init) out += " = " + genExpr(vd->init);
                } else if (auto es = dynamic_pointer_cast<ExprStmt>(fs->init)) {
                    out += genExpr(es->expr);
                }
            }
            out += "; ";
            if (fs->condition) out += genExpr(fs->condition);
            out += "; ";
            if (fs->update) out += genExpr(fs->update);
            out += ") {\n";
            indentLevel++;
            out += genBlock(fs->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
            out += indent() + "for (auto " + fe->varName + " : " + genExpr(fe->iterable) + ") {\n";
            indentLevel++;
            out += genBlock(fe->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
            out += indent() + "switch (" + genExpr(sw->expr) + ") {\n";
            indentLevel++;
            for (auto& c : sw->cases) {
                if (c.value) {
                    out += indent() + "case " + genExpr(c.value) + ":\n";
                } else {
                    out += indent() + "default:\n";
                }
                indentLevel++;
                for (auto& st : c.body) out += genStmt(st);
                indentLevel--;
            }
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (dynamic_pointer_cast<BreakStmt>(s)) {
            out += indent() + "break;\n";
        }
        else if (dynamic_pointer_cast<ContinueStmt>(s)) {
            out += indent() + "continue;\n";
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            out += indent() + "return";
            if (rs->value) out += " " + genExpr(rs->value);
            out += ";\n";
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            string rt = mapTypeToCpp(fd->returnType);
            if (rt.empty()) rt = "void"; // constructor fallback
            out += indent() + rt + " " + fd->name + "(";
            for (size_t i = 0; i < fd->params.size(); i++) {
                if (i > 0) out += ", ";
                string pt = mapTypeToCpp(fd->params[i].type);
                if (fd->params[i].isArray) {
                    usesVector = true;
                    pt = "vector<" + pt + ">";
                }
                out += pt + " " + fd->params[i].name;
            }
            out += ") {\n";
            indentLevel++;
            if (fd->body) {
                for (auto& st : fd->body->stmts) out += genStmt(st);
            }
            indentLevel--;
            out += indent() + "}\n\n";
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            inClass = true;
            out += indent() + "class " + cd->name;
            if (!cd->parent.empty()) out += " : public " + cd->parent;
            out += " {\n";

            // Group by access
            map<string, vector<ClassDecl::Field>> fieldsByAccess;
            map<string, vector<shared_ptr<FunctionDecl>>> methodsByAccess;
            for (auto& f : cd->fields) fieldsByAccess[f.access].push_back(f);
            for (auto& m : cd->methods) methodsByAccess[m->access].push_back(m);

            vector<string> accessOrder = {"public", "protected", "private"};
            for (auto& acc : accessOrder) {
                bool hasFields = fieldsByAccess.count(acc) && !fieldsByAccess[acc].empty();
                bool hasMethods = methodsByAccess.count(acc) && !methodsByAccess[acc].empty();
                bool hasCtors = (acc == "public" && !cd->constructors.empty());
                if (!hasFields && !hasMethods && !hasCtors) continue;

                out += indent() + acc + ":\n";
                indentLevel++;

                if (fieldsByAccess.count(acc)) {
                    for (auto& f : fieldsByAccess[acc]) {
                        out += indent() + mapTypeToCpp(f.type) + " " + f.name;
                        if (f.init) out += " = " + genExpr(f.init);
                        out += ";\n";
                    }
                }

                if (acc == "public") {
                    for (auto& ctor : cd->constructors) {
                        out += indent() + cd->name + "(";
                        for (size_t i = 0; i < ctor->params.size(); i++) {
                            if (i > 0) out += ", ";
                            out += mapTypeToCpp(ctor->params[i].type) + " " + ctor->params[i].name;
                        }
                        out += ") {\n";
                        indentLevel++;
                        if (ctor->body) {
                            for (auto& st : ctor->body->stmts) out += genStmt(st);
                        }
                        indentLevel--;
                        out += indent() + "}\n\n";
                    }
                }

                if (methodsByAccess.count(acc)) {
                    for (auto& m : methodsByAccess[acc]) {
                        out += genStmt(m);
                    }
                }
                indentLevel--;
            }

            out += indent() + "};\n\n";
            inClass = false;
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            out += indent() + "try {\n";
            indentLevel++;
            if (tc->tryBlock) for (auto& st : tc->tryBlock->stmts) out += genStmt(st);
            indentLevel--;
            out += indent() + "}";

            if (tc->catchBlock) {
                out += " catch (exception& " + tc->catchVar + ") {\n";
                indentLevel++;
                for (auto& st : tc->catchBlock->stmts) out += genStmt(st);
                indentLevel--;
                out += indent() + "}";
            }
            // C++ doesn't have finally, simulate with scope or just comment
            if (tc->finallyBlock) {
                out += "\n" + indent() + "// finally block\n";
                out += indent() + "{\n";
                indentLevel++;
                for (auto& st : tc->finallyBlock->stmts) out += genStmt(st);
                indentLevel--;
                out += indent() + "}";
            }
            out += "\n";
        }
        else if (auto th = dynamic_pointer_cast<ThrowStmt>(s)) {
            out += indent() + "throw runtime_error(" + genExpr(th->expr) + ");\n";
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            for (auto& st : bs->stmts) out += genStmt(st);
        }
        else if (auto imp = dynamic_pointer_cast<ImportStmt>(s)) {
            // For C++, keep includes if they look like C++ includes
            string path = imp->path;
            if (path.find(".") == string::npos && !path.empty()) {
                // Looks like a C++ header
                out += "#include <" + path + ">\n";
            }
        }

        return out;
    }

    string genBlock(StmtPtr s) {
        if (!s) return "";
        if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            string out = "";
            for (auto& st : bs->stmts) out += genStmt(st);
            return out;
        }
        return genStmt(s);
    }

    string generate(vector<StmtPtr>& program) {
        indentLevel = 0;
        usesInput = false;
        usesVector = false;
        usesMap = false;
        declaredVars.clear();
        string body = "";

        // Separate declarations
        vector<StmtPtr> imports, classes, functions, mainStmts;
        for (auto& s : program) {
            if (dynamic_pointer_cast<ImportStmt>(s)) imports.push_back(s);
            else if (dynamic_pointer_cast<ClassDecl>(s)) classes.push_back(s);
            else if (dynamic_pointer_cast<FunctionDecl>(s)) functions.push_back(s);
            else mainStmts.push_back(s);
        }

        // Pre-scan for features
        string classBody = "", funcBody = "", mainBody = "";

        for (auto& s : classes) classBody += genStmt(s);
        for (auto& s : functions) funcBody += genStmt(s);

        indentLevel = 1;
        for (auto& s : mainStmts) mainBody += genStmt(s);
        indentLevel = 0;

        // Build output with headers
        string out = "";
        out += "#include <iostream>\n";
        out += "#include <string>\n";
        if (usesVector) out += "#include <vector>\n";
        if (usesMap) out += "#include <map>\n";
        out += "#include <stdexcept>\n";
        out += "using namespace std;\n\n";

        // Imports
        for (auto& s : imports) out += genStmt(s);

        // Classes
        out += classBody;

        // Functions
        out += funcBody;

        // Main function
        out += "int main() {\n";
        out += mainBody;
        out += "    return 0;\n";
        out += "}\n";

        return out;
    }
};

// ==========================================================================
// JAVA GENERATOR
// ==========================================================================

class JavaGenerator {
    int indentLevel = 0;
    string indent() { return string(indentLevel * 4, ' '); }
    bool usesScanner = false;
    bool usesArrayList = false;
    bool usesHashMap = false;
    unordered_set<string> declaredVars;
    bool inClass = false;

public:
    string genExpr(ExprPtr e) {
        if (!e) return "";
        if (auto n = dynamic_pointer_cast<NumberLiteral>(e)) return to_string(n->value);
        if (auto f = dynamic_pointer_cast<FloatLiteral>(e)) {
            ostringstream oss; oss << f->value; return oss.str();
        }
        if (auto s = dynamic_pointer_cast<StringLiteral>(e)) return "\"" + s->value + "\"";
        if (auto b = dynamic_pointer_cast<BoolLiteral>(e)) return b->value ? "true" : "false";
        if (auto c = dynamic_pointer_cast<CharLiteral>(e)) return "'" + c->value + "'";
        if (dynamic_pointer_cast<NullLiteral>(e)) return "null";
        if (auto v = dynamic_pointer_cast<Variable>(e)) return v->name;
        if (dynamic_pointer_cast<ThisExpr>(e)) return "this";

        if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
            return genExpr(bin->left) + " " + bin->op + " " + genExpr(bin->right);
        }
        if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
            if (un->isPrefix) return un->op + genExpr(un->operand);
            else return genExpr(un->operand) + un->op;
        }
        if (auto ae = dynamic_pointer_cast<AssignExpr>(e)) {
            return genExpr(ae->target) + " " + ae->op + " " + genExpr(ae->value);
        }
        if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
            return genExpr(te->condition) + " ? " + genExpr(te->thenExpr) + " : " + genExpr(te->elseExpr);
        }
        if (auto ce = dynamic_pointer_cast<CallExpr>(e)) {
            string callee = genExpr(ce->callee);
            string args = "";
            for (size_t i = 0; i < ce->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ce->args[i]);
            }
            return callee + "(" + args + ")";
        }
        if (auto ie = dynamic_pointer_cast<IndexExpr>(e)) {
            return genExpr(ie->object) + "[" + genExpr(ie->index) + "]";
        }
        if (auto me = dynamic_pointer_cast<MemberExpr>(e)) {
            return genExpr(me->object) + "." + me->member;
        }
        if (auto ne = dynamic_pointer_cast<NewExpr>(e)) {
            string args = "";
            for (size_t i = 0; i < ne->args.size(); i++) {
                if (i > 0) args += ", ";
                args += genExpr(ne->args[i]);
            }
            return "new " + ne->className + "(" + args + ")";
        }
        if (auto al = dynamic_pointer_cast<ArrayLitExpr>(e)) {
            string elems = "";
            for (size_t i = 0; i < al->elements.size(); i++) {
                if (i > 0) elems += ", ";
                elems += genExpr(al->elements[i]);
            }
            return "{" + elems + "}";
        }
        if (auto inp = dynamic_pointer_cast<InputExpr>(e)) {
            usesScanner = true;
            return "sc.nextLine()";
        }
        if (auto cast = dynamic_pointer_cast<CastExpr>(e)) {
            return "(" + mapTypeToJava(cast->targetType) + ") " + genExpr(cast->expr);
        }
        return "";
    }

    string genStmt(StmtPtr s) {
        if (!s) return "";
        string out = "";

        if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
            string type = mapTypeToJava(resolveAutoType(vd->type, vd->init));
            if (vd->isArray) {
                usesArrayList = true;
                type = type + "[]";
            }
            out += indent();
            if (vd->init && dynamic_pointer_cast<InputExpr>(vd->init)) {
                usesScanner = true;
                if (type == "int") {
                    out += type + " " + vd->name + " = sc.nextInt();\n";
                } else if (type == "double" || type == "float") {
                    out += type + " " + vd->name + " = sc.nextDouble();\n";
                } else {
                    out += type + " " + vd->name + " = sc.nextLine();\n";
                }
            } else {
                out += type + " " + vd->name;
                if (vd->init) out += " = " + genExpr(vd->init);
                out += ";\n";
            }
            declaredVars.insert(vd->name);
        }
        else if (auto es = dynamic_pointer_cast<ExprStmt>(s)) {
            if (auto ae = dynamic_pointer_cast<AssignExpr>(es->expr)) {
                if (dynamic_pointer_cast<InputExpr>(ae->value)) {
                    usesScanner = true;
                    out += indent() + genExpr(ae->target) + " = sc.nextLine();\n";
                    return out;
                }
            }
            out += indent() + genExpr(es->expr) + ";\n";
        }
        else if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
            out += indent() + "System.out.println(";
            for (size_t i = 0; i < ps->args.size(); i++) {
                if (i > 0) out += " + \" \" + ";
                out += genExpr(ps->args[i]);
            }
            out += ");\n";
        }
        else if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
            out += indent() + "if (" + genExpr(is->condition) + ") {\n";
            indentLevel++;
            out += genBlock(is->thenBranch);
            indentLevel--;
            out += indent() + "}";
            if (is->elseBranch) {
                if (auto elseIf = dynamic_pointer_cast<IfStmt>(is->elseBranch)) {
                    out += " else if (" + genExpr(elseIf->condition) + ") {\n";
                    indentLevel++;
                    out += genBlock(elseIf->thenBranch);
                    indentLevel--;
                    out += indent() + "}";
                    auto eb = elseIf->elseBranch;
                    while (eb) {
                        if (auto ei = dynamic_pointer_cast<IfStmt>(eb)) {
                            out += " else if (" + genExpr(ei->condition) + ") {\n";
                            indentLevel++;
                            out += genBlock(ei->thenBranch);
                            indentLevel--;
                            out += indent() + "}";
                            eb = ei->elseBranch;
                        } else {
                            out += " else {\n";
                            indentLevel++;
                            out += genBlock(eb);
                            indentLevel--;
                            out += indent() + "}";
                            break;
                        }
                    }
                } else {
                    out += " else {\n";
                    indentLevel++;
                    out += genBlock(is->elseBranch);
                    indentLevel--;
                    out += indent() + "}";
                }
            }
            out += "\n";
        }
        else if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
            out += indent() + "while (" + genExpr(ws->condition) + ") {\n";
            indentLevel++;
            out += genBlock(ws->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
            out += indent() + "do {\n";
            indentLevel++;
            out += genBlock(dw->body);
            indentLevel--;
            out += indent() + "} while (" + genExpr(dw->condition) + ");\n";
        }
        else if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
            out += indent() + "for (";
            if (fs->init) {
                if (auto vd = dynamic_pointer_cast<VarDeclStmt>(fs->init)) {
                    out += mapTypeToJava(resolveAutoType(vd->type, vd->init)) + " " + vd->name;
                    if (vd->init) out += " = " + genExpr(vd->init);
                } else if (auto es = dynamic_pointer_cast<ExprStmt>(fs->init)) {
                    out += genExpr(es->expr);
                }
            }
            out += "; ";
            if (fs->condition) out += genExpr(fs->condition);
            out += "; ";
            if (fs->update) out += genExpr(fs->update);
            out += ") {\n";
            indentLevel++;
            out += genBlock(fs->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
            string type = mapTypeToJava(fe->type);
            out += indent() + "for (" + type + " " + fe->varName + " : " + genExpr(fe->iterable) + ") {\n";
            indentLevel++;
            out += genBlock(fe->body);
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
            out += indent() + "switch (" + genExpr(sw->expr) + ") {\n";
            indentLevel++;
            for (auto& c : sw->cases) {
                if (c.value) out += indent() + "case " + genExpr(c.value) + ":\n";
                else out += indent() + "default:\n";
                indentLevel++;
                for (auto& st : c.body) out += genStmt(st);
                indentLevel--;
            }
            indentLevel--;
            out += indent() + "}\n";
        }
        else if (dynamic_pointer_cast<BreakStmt>(s)) {
            out += indent() + "break;\n";
        }
        else if (dynamic_pointer_cast<ContinueStmt>(s)) {
            out += indent() + "continue;\n";
        }
        else if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
            out += indent() + "return";
            if (rs->value) out += " " + genExpr(rs->value);
            out += ";\n";
        }
        else if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
            string rt = mapTypeToJava(fd->returnType);
            if (rt.empty()) rt = "void";
            out += indent();
            if (!inClass) out += "public static ";
            else {
                if (fd->access == "private") out += "private ";
                else if (fd->access == "protected") out += "protected ";
                else out += "public ";
                if (fd->isStatic) out += "static ";
            }
            out += rt + " " + fd->name + "(";
            for (size_t i = 0; i < fd->params.size(); i++) {
                if (i > 0) out += ", ";
                string pt = mapTypeToJava(fd->params[i].type);
                if (fd->params[i].isArray) pt += "[]";
                out += pt + " " + fd->params[i].name;
            }
            out += ") {\n";
            indentLevel++;
            if (fd->body) {
                for (auto& st : fd->body->stmts) out += genStmt(st);
            }
            indentLevel--;
            out += indent() + "}\n\n";
        }
        else if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
            bool wasInClass = inClass;
            inClass = true;
            out += indent() + "static class " + cd->name;
            if (!cd->parent.empty()) out += " extends " + cd->parent;
            out += " {\n";
            indentLevel++;

            // Fields
            for (auto& f : cd->fields) {
                out += indent();
                if (f.access == "private") out += "private ";
                else if (f.access == "protected") out += "protected ";
                else out += "public ";
                if (f.isStatic) out += "static ";
                out += mapTypeToJava(f.type) + " " + f.name;
                if (f.init) out += " = " + genExpr(f.init);
                out += ";\n";
            }
            if (!cd->fields.empty()) out += "\n";

            // Constructors
            for (auto& ctor : cd->constructors) {
                out += indent() + "public " + cd->name + "(";
                for (size_t i = 0; i < ctor->params.size(); i++) {
                    if (i > 0) out += ", ";
                    out += mapTypeToJava(ctor->params[i].type) + " " + ctor->params[i].name;
                }
                out += ") {\n";
                indentLevel++;
                if (ctor->body) {
                    for (auto& st : ctor->body->stmts) out += genStmt(st);
                }
                indentLevel--;
                out += indent() + "}\n\n";
            }

            // Methods
            for (auto& m : cd->methods) {
                out += genStmt(m);
            }

            indentLevel--;
            out += indent() + "}\n\n";
            inClass = wasInClass;
        }
        else if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
            out += indent() + "try {\n";
            indentLevel++;
            if (tc->tryBlock) for (auto& st : tc->tryBlock->stmts) out += genStmt(st);
            indentLevel--;
            out += indent() + "}";

            if (tc->catchBlock) {
                out += " catch (" + tc->catchType + " " + tc->catchVar + ") {\n";
                indentLevel++;
                for (auto& st : tc->catchBlock->stmts) out += genStmt(st);
                indentLevel--;
                out += indent() + "}";
            }

            if (tc->finallyBlock) {
                out += " finally {\n";
                indentLevel++;
                for (auto& st : tc->finallyBlock->stmts) out += genStmt(st);
                indentLevel--;
                out += indent() + "}";
            }
            out += "\n";
        }
        else if (auto th = dynamic_pointer_cast<ThrowStmt>(s)) {
            out += indent() + "throw new RuntimeException(" + genExpr(th->expr) + ");\n";
        }
        else if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            for (auto& st : bs->stmts) out += genStmt(st);
        }
        else if (auto imp = dynamic_pointer_cast<ImportStmt>(s)) {
            string path = imp->path;
            if (path.find("java") != string::npos) {
                out += "import " + path + ";\n";
            }
        }

        return out;
    }

    string genBlock(StmtPtr s) {
        if (!s) return "";
        if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
            string out = "";
            for (auto& st : bs->stmts) out += genStmt(st);
            return out;
        }
        return genStmt(s);
    }

    string generate(vector<StmtPtr>& program) {
        indentLevel = 0;
        usesScanner = false;
        usesArrayList = false;
        usesHashMap = false;
        declaredVars.clear();
        inClass = false;

        // Separate declarations
        vector<StmtPtr> imports, classes, functions, mainStmts;
        for (auto& s : program) {
            if (dynamic_pointer_cast<ImportStmt>(s)) imports.push_back(s);
            else if (dynamic_pointer_cast<ClassDecl>(s)) classes.push_back(s);
            else if (dynamic_pointer_cast<FunctionDecl>(s)) functions.push_back(s);
            else mainStmts.push_back(s);
        }

        // Pre-generate to detect features
        string classBody = "", funcBody = "", mainBody = "";

        indentLevel = 1;
        for (auto& s : classes) classBody += genStmt(s);
        for (auto& s : functions) funcBody += genStmt(s);

        indentLevel = 2;
        for (auto& s : mainStmts) mainBody += genStmt(s);
        indentLevel = 0;

        // Build output
        string out = "";
        out += "import java.util.*;\n";
        if (usesScanner) out += "import java.util.Scanner;\n";
        out += "\n";

        out += "public class Main {\n";

        // Inner classes
        out += classBody;

        // Static methods
        out += funcBody;

        // Main method
        out += "    public static void main(String[] args) {\n";
        if (usesScanner) {
            out += "        Scanner sc = new Scanner(System.in);\n";
        }
        out += mainBody;
        out += "    }\n";
        out += "}\n";

        return out;
    }
};

// ============================================================================
// SECTION 9: AST / IR SERIALIZATION
// ============================================================================

string serializeExpr(ExprPtr e, int depth = 0) {
    string pad(depth * 2, ' ');
    if (!e) return pad + "(null)\n";

    if (auto n = dynamic_pointer_cast<NumberLiteral>(e))
        return pad + "Number(" + to_string(n->value) + ")\n";
    if (auto f = dynamic_pointer_cast<FloatLiteral>(e)) {
        ostringstream oss; oss << f->value;
        return pad + "Float(" + oss.str() + ")\n";
    }
    if (auto s = dynamic_pointer_cast<StringLiteral>(e))
        return pad + "String(\"" + s->value + "\")\n";
    if (auto b = dynamic_pointer_cast<BoolLiteral>(e))
        return pad + "Bool(" + (b->value ? "true" : "false") + ")\n";
    if (auto c = dynamic_pointer_cast<CharLiteral>(e))
        return pad + "Char('" + c->value + "')\n";
    if (dynamic_pointer_cast<NullLiteral>(e))
        return pad + "Null\n";
    if (auto v = dynamic_pointer_cast<Variable>(e))
        return pad + "Var(" + v->name + ")\n";
    if (dynamic_pointer_cast<ThisExpr>(e))
        return pad + "This\n";
    if (auto bin = dynamic_pointer_cast<BinaryExpr>(e)) {
        string out = pad + "BinaryExpr(" + bin->op + ")\n";
        out += serializeExpr(bin->left, depth + 1);
        out += serializeExpr(bin->right, depth + 1);
        return out;
    }
    if (auto un = dynamic_pointer_cast<UnaryExpr>(e)) {
        string out = pad + "UnaryExpr(" + un->op + ", " + (un->isPrefix ? "pre" : "post") + ")\n";
        out += serializeExpr(un->operand, depth + 1);
        return out;
    }
    if (auto ae = dynamic_pointer_cast<AssignExpr>(e)) {
        string out = pad + "Assign(" + ae->op + ")\n";
        out += serializeExpr(ae->target, depth + 1);
        out += serializeExpr(ae->value, depth + 1);
        return out;
    }
    if (auto ce = dynamic_pointer_cast<CallExpr>(e)) {
        string out = pad + "Call\n";
        out += serializeExpr(ce->callee, depth + 1);
        for (auto& a : ce->args) out += serializeExpr(a, depth + 1);
        return out;
    }
    if (auto me = dynamic_pointer_cast<MemberExpr>(e))
        return pad + "Member(." + me->member + ")\n" + serializeExpr(me->object, depth + 1);
    if (auto ie = dynamic_pointer_cast<IndexExpr>(e)) {
        return pad + "Index\n" + serializeExpr(ie->object, depth + 1) + serializeExpr(ie->index, depth + 1);
    }
    if (auto ne = dynamic_pointer_cast<NewExpr>(e)) {
        string out = pad + "New(" + ne->className + ")\n";
        for (auto& a : ne->args) out += serializeExpr(a, depth + 1);
        return out;
    }
    if (auto al = dynamic_pointer_cast<ArrayLitExpr>(e)) {
        string out = pad + "ArrayLit\n";
        for (auto& el : al->elements) out += serializeExpr(el, depth + 1);
        return out;
    }
    if (auto te = dynamic_pointer_cast<TernaryExpr>(e)) {
        return pad + "Ternary\n" + serializeExpr(te->condition, depth + 1)
            + serializeExpr(te->thenExpr, depth + 1) + serializeExpr(te->elseExpr, depth + 1);
    }
    if (dynamic_pointer_cast<InputExpr>(e))
        return pad + "Input()\n";
    return pad + "(expr)\n";
}

string serializeStmt(StmtPtr s, int depth = 0) {
    string pad(depth * 2, ' ');
    if (!s) return "";

    if (auto vd = dynamic_pointer_cast<VarDeclStmt>(s)) {
        string out = pad + "VarDecl(" + vd->type + " " + vd->name + ")\n";
        if (vd->init) out += serializeExpr(vd->init, depth + 1);
        return out;
    }
    if (auto es = dynamic_pointer_cast<ExprStmt>(s))
        return pad + "ExprStmt\n" + serializeExpr(es->expr, depth + 1);
    if (auto ps = dynamic_pointer_cast<PrintStmt>(s)) {
        string out = pad + "Print\n";
        for (auto& a : ps->args) out += serializeExpr(a, depth + 1);
        return out;
    }
    if (auto is = dynamic_pointer_cast<IfStmt>(s)) {
        string out = pad + "If\n";
        out += serializeExpr(is->condition, depth + 1);
        out += pad + "  Then:\n" + serializeStmt(is->thenBranch, depth + 2);
        if (is->elseBranch) out += pad + "  Else:\n" + serializeStmt(is->elseBranch, depth + 2);
        return out;
    }
    if (auto ws = dynamic_pointer_cast<WhileStmt>(s)) {
        return pad + "While\n" + serializeExpr(ws->condition, depth + 1) + serializeStmt(ws->body, depth + 1);
    }
    if (auto dw = dynamic_pointer_cast<DoWhileStmt>(s)) {
        return pad + "DoWhile\n" + serializeStmt(dw->body, depth + 1) + serializeExpr(dw->condition, depth + 1);
    }
    if (auto fs = dynamic_pointer_cast<ForStmt>(s)) {
        string out = pad + "For\n";
        if (fs->init) out += pad + "  Init:\n" + serializeStmt(fs->init, depth + 2);
        if (fs->condition) out += pad + "  Cond:\n" + serializeExpr(fs->condition, depth + 2);
        if (fs->update) out += pad + "  Update:\n" + serializeExpr(fs->update, depth + 2);
        out += pad + "  Body:\n" + serializeStmt(fs->body, depth + 2);
        return out;
    }
    if (auto fe = dynamic_pointer_cast<ForEachStmt>(s)) {
        return pad + "ForEach(" + fe->type + " " + fe->varName + ")\n" +
               serializeExpr(fe->iterable, depth + 1) + serializeStmt(fe->body, depth + 1);
    }
    if (auto sw = dynamic_pointer_cast<SwitchStmt>(s)) {
        string out = pad + "Switch\n" + serializeExpr(sw->expr, depth + 1);
        for (auto& c : sw->cases) {
            if (c.value) out += pad + "  Case:\n" + serializeExpr(c.value, depth + 2);
            else out += pad + "  Default:\n";
            for (auto& st : c.body) out += serializeStmt(st, depth + 2);
        }
        return out;
    }
    if (dynamic_pointer_cast<BreakStmt>(s)) return pad + "Break\n";
    if (dynamic_pointer_cast<ContinueStmt>(s)) return pad + "Continue\n";
    if (auto rs = dynamic_pointer_cast<ReturnStmt>(s)) {
        string out = pad + "Return\n";
        if (rs->value) out += serializeExpr(rs->value, depth + 1);
        return out;
    }
    if (auto fd = dynamic_pointer_cast<FunctionDecl>(s)) {
        string out = pad + "Function(" + fd->returnType + " " + fd->name + ")\n";
        for (auto& p : fd->params) out += pad + "  Param(" + p.type + " " + p.name + ")\n";
        if (fd->body) out += serializeStmt(fd->body, depth + 1);
        return out;
    }
    if (auto cd = dynamic_pointer_cast<ClassDecl>(s)) {
        string out = pad + "Class(" + cd->name;
        if (!cd->parent.empty()) out += " extends " + cd->parent;
        out += ")\n";
        for (auto& f : cd->fields) out += pad + "  Field(" + f.access + " " + f.type + " " + f.name + ")\n";
        for (auto& c : cd->constructors) out += serializeStmt(c, depth + 1);
        for (auto& m : cd->methods) out += serializeStmt(m, depth + 1);
        return out;
    }
    if (auto tc = dynamic_pointer_cast<TryCatchStmt>(s)) {
        string out = pad + "TryCatch\n";
        if (tc->tryBlock) out += pad + "  Try:\n" + serializeStmt(tc->tryBlock, depth + 2);
        if (tc->catchBlock) out += pad + "  Catch(" + tc->catchType + " " + tc->catchVar + "):\n" + serializeStmt(tc->catchBlock, depth + 2);
        if (tc->finallyBlock) out += pad + "  Finally:\n" + serializeStmt(tc->finallyBlock, depth + 2);
        return out;
    }
    if (auto th = dynamic_pointer_cast<ThrowStmt>(s))
        return pad + "Throw\n" + serializeExpr(th->expr, depth + 1);
    if (auto bs = dynamic_pointer_cast<BlockStmt>(s)) {
        string out = pad + "Block\n";
        for (auto& st : bs->stmts) out += serializeStmt(st, depth + 1);
        return out;
    }
    if (auto imp = dynamic_pointer_cast<ImportStmt>(s))
        return pad + "Import(" + imp->path + ")\n";

    return pad + "(stmt)\n";
}

string serializeAST(vector<StmtPtr>& program) {
    string out = "Program\n";
    for (auto& s : program) out += serializeStmt(s, 1);
    return out;
}

string serializeIR(vector<IR>& code) {
    string out = "";
    for (size_t i = 0; i < code.size(); i++) {
        out += "[" + to_string(i) + "] " + code[i].op;
        if (!code[i].arg1.empty()) out += " " + code[i].arg1;
        if (!code[i].arg2.empty()) out += " " + code[i].arg2;
        if (!code[i].res.empty()) out += " -> " + code[i].res;
        out += "\n";
    }
    return out;
}

// ============================================================================
// SECTION 10: JSON OUTPUT & MAIN
// ============================================================================

string escapeJson(const string& s) {
    string out;
    out.reserve(s.size() + 32);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);

    string input = "", line;

    while (true) {
        if (!getline(cin, line)) break;
        if (line.find("###END###") != string::npos) break;
        input += line + "\n";
    }

    string target;
    getline(cin, target);

    // Trim target
    while (!target.empty() && isspace(target.back())) target.pop_back();
    while (!target.empty() && isspace(target.front())) target.erase(target.begin());

    string outputCode = "";
    string astStr = "";
    string irStr = "";
    string optimizedIrStr = "";
    vector<string> errors;

    try {
        // 1. Lexing
        Lexer lexer(input);
        auto tokens = lexer.tokenize();

        // 2. Parsing
        Parser parser(tokens);
        auto program = parser.parse();

        // 3. AST serialization (before optimization)
        astStr = serializeAST(program);

        // 4. Semantic Analysis
        SemanticAnalyzer analyzer;
        auto semErrors = analyzer.analyze(program);
        for (auto& e : semErrors) errors.push_back(e);

        // 5. Optimization (AST level)
        Optimizer::optimize(program);

        // 6. IR Generation
        IRGenerator irGen;
        auto ir = irGen.generate(program);
        irStr = serializeIR(ir);

        // 7. IR Optimization
        auto optIr = Optimizer::optimizeIR(ir);
        optimizedIrStr = serializeIR(optIr);

        // 8. Code Generation
        if (target == "python") {
            PythonGenerator gen;
            outputCode = gen.generate(program);
        } else if (target == "cpp") {
            CppGenerator gen;
            outputCode = gen.generate(program);
        } else if (target == "java") {
            JavaGenerator gen;
            outputCode = gen.generate(program);
        } else {
            errors.push_back("Unknown target language: " + target);
        }
    } catch (exception& e) {
        errors.push_back(string("Compiler error: ") + e.what());
    }

    // Output as JSON
    cout << "{";
    cout << "\"output\":\"" << escapeJson(outputCode) << "\",";
    cout << "\"ast\":\"" << escapeJson(astStr) << "\",";
    cout << "\"ir\":\"" << escapeJson(irStr) << "\",";
    cout << "\"optimized_ir\":\"" << escapeJson(optimizedIrStr) << "\",";
    cout << "\"errors\":[";
    for (size_t i = 0; i < errors.size(); i++) {
        if (i > 0) cout << ",";
        cout << "\"" << escapeJson(errors[i]) << "\"";
    }
    cout << "]";
    cout << "}";

    return 0;
}
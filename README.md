# Multi-Target Source-to-Source Compiler

A multi-target source-to-source compiler/transpiler that converts a C-like input language to **Python**, **C++**, and **Java**.

## Project Status

Current stage: Active Development

Implemented:

- [x] Lexer with 60+ token types
- [x] Recursive descent parser
- [x] AST generation
- [x] Semantic analysis
- [x] IR generation
- [x] Basic optimizations
- [x] Python/C++/Java code generation

Under development:

- [ ] Advanced OOP support
- [ ] Constructor parsing
- [ ] Complex object initialization
- [ ] Full library mapping
- [ ] Advanced function signatures

## Supported

- Variables and primitive types
- Arithmetic expressions
- If/Else
- While loops
- For loops
- Recursive functions
- Arrays
- Exception handling
- Basic OOP
- Semantic analysis
- IR generation

## Planned

Constructor initialization
Full inheritance handling
Templates/generics
Async/threading
Complete standard library mapping
Advanced object expressions

## Project Structure

```text
multitarget-compiler-transpiler/
│
├── compiler.cpp        # Core compiler implementation
├── app.py              # Flask web application
├── README.md           # Project documentation
├── .gitignore
│
├── screenshots/
│   ├── ui.png
│   ├── ast.png
│   ├── ir.png
│   └── optimized_ir.png
│
├── templates/
│   └── index.html      # Frontend page
│
├── static/
│   └── style.css       # UI styling
│
├── tests/
│   ├── test_binary_search.txt
│   ├── test_control_flow.txt
│   ├── test_datatypes.txt
│   ├── test_exception.txt
│   ├── test_linked_list.txt
│   ├── test_oop.txt
│   ├── test_recursive.txt
│   └── test_stack.txt
```

## Architecture

```text
                    Input Source Code
                            │
                            ▼
┌─────────────────────────────────────────┐
│ Lexer                                   │
│ • 60+ token types                       │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ Parser                                  │
│ • Recursive descent                     │
│ • Full operator precedence              │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ AST                                     │
│ • 30+ node types                        │
│ • shared_ptr hierarchy                  │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ Semantic Analyzer                       │
│ • Symbol table                          │
│ • Scope checking                        │
│ • Type inference                        │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ IR Generator                            │
│ • Extended IR generation                │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ Optimizer                               │
│ • Constant folding                      │
│ • Dead code elimination                 │
│ • Copy propagation                      │
│ • Algebraic simplification              │
└─────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────┐
│ Code Generator                          │
│ • Python generation                     │
│ • C++ generation                        │
│ • Java generation                       │
└─────────────────────────────────────────┘
                            │
                            ▼
                     Output Source Code
```

## Example Conversion

Input:

```c
let x=5;
print(x);
```

Output (Python):

```python
x=5
print(x)
```

Output (C++):

```cpp
#include <iostream>
using namespace std;

int main(){

int x=5;

cout<<x<<endl;

return 0;

}
```

Output (Java):

```java
public class Main{

public static void main(String[] args){

int x=5;

System.out.println(x);

}

}
```

## Contributing

Contributions are welcome.

Steps:

1. Fork the repository
2. Create a feature branch

```bash
git checkout -b feature-name
```

3. Commit changes

```bash
git commit -m "Added feature"
```

4. Push changes

```bash
git push origin feature-name
```

5. Open a Pull Request


## Screenshots

### Web Interface
![Web UI](screenshots/ui.png)

### AST View
![AST](screenshots/ast.png)

### IR View
![IR](screenshots/ir.png)

### Optimized IR
![Optimized IR](screenshots/optimized_ir.png)

```
## Build

```bash
g++ -std=c++17 compiler.cpp -o compiler.exe
```

## Run Web App

Install dependencies:

```bash
pip install flask
```

Run the application:

```bash
python app.py
```

Open in your browser:

```text
http://127.0.0.1:5000
```

Note: This URL runs locally on your machine. Other users must run the application themselves to access the interface.

## Supported Features

### Variables & Types
- [x] int
- [x] float
- [x] double
- [x] string
- [x] bool
- [x] char
- [x] Type inference (`let`)

### Control Flow
- [x] if / else if / else
- [x] while loops
- [x] do-while loops
- [x] for loops
- [x] switch / case / default
- [x] break / continue
- [x] ternary operator

### Functions
- [x] Function definitions
- [x] Recursive functions
- [x] Return statements

### Object-Oriented Programming
- [x] Basic classes
- [x] Methods
- [x] Member variables
- [ ] Advanced constructors
- [ ] Full inheritance support

### Exception Handling
- [x] try
- [x] catch
- [x] finally

### Compiler Pipeline
- [x] Lexer
- [x] Recursive descent parser
- [x] AST generation
- [x] Semantic analysis
- [x] IR generation
- [x] Constant folding
- [x] Dead code elimination
- [x] Python code generation
- [x] C++ code generation
- [x] Java code generation

### Variables & Types
```
int x = 10;
float y = 2.5;
double pi = 3.14;
string name = "hello";
bool flag = true;
char ch = 'A';
let auto_typed = 42;    // type inferred
```

### Control Flow
```
if / else if / else
while loops
do-while loops
for loops (C-style)
for-each loops
switch / case / default
break / continue
ternary operator (? :)
```

### Functions
```
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

### OOP
```
class Dog extends Animal {
    private:
    string name;

    public:
    Dog(string n) { this.name = n; }
    void speak() { print("Woof!"); }
}
```

### Exception Handling
```
try {
    int x = divide(10, 0);
} catch (Exception e) {
    print("Error!");
} finally {
    print("Done");
}
```

### Other
- Arrays: `int[] arr = {1, 2, 3};`
- Input: `let x = input();`
- Print: `print(x);`
- Comments: `// line` and `/* block */`
- Imports: `#include<iostream>` / `import java.util.*;`
- Operators: `== != < > <= >= && || ! ++ -- += -=`

## Test

```bash
echo "let x = 5; print(x);" > test.txt
echo "###END###" >> test.txt
echo "python" >> test.txt
type test.txt | compiler.exe
```

## License

MIT License

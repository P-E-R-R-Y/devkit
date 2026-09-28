# {{name}}

**P-E-R-R-Y {{name}}**

<!-- one line: what this repository is for -->

[![Docs](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://p-e-r-r-y.github.io/{{name}})
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

---

## ✨ Overview

<!-- what it does, and what it refuses to do -->

Core goals:
- 🧩
- ⚡
- ✅ **Unit-tested** with GoogleTest

---

## 🧱 Features

<!-- one line per capability, written as the user sees it -->

-
-

---

## 🧩 Example

```cpp
#include "{{name}}.hpp"

int main() {
}
```

---

## 🔧 Build

```sh
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure
```

Add it to another P-E-R-R-Y repository with:

```sh
devkit deps add {{name}} -v {{version}}
```

# Custom String Class — C++ Laboratory Work

## 📌 Description
This laboratory work implements a fully custom `String` class in C++ without using `<string>` or `<cstring>`.  
The class supports dynamic memory management, deep copying, move semantics, safe indexing with exceptions, concatenation, comparisons, `reserve`, `push_back`, and a function for extracting unique characters between two strings.

The project demonstrates understanding of low‑level memory handling, RAII, exception safety, and operator overloading.

---

## 🛠 Implemented Features
- Default constructor, constructor from C‑string  
- Copy constructor and move constructor  
- Copy assignment and move assignment  
- Safe `operator[]` with `std::out_of_range`  
- `c_str()`, `length()`, `empty()`  
- `reserve()` and `push_back()`  
- Operators `+`, `+=` (String and C‑string)  
- Lexicographical comparisons  
- `clear()`  
- `unique_chars_with()` — returns characters that appear only in one of the two strings  

---

## 🎯 Educational Goals
- Practice manual memory management  
- Understand deep vs shallow copying  
- Implement move semantics  
- Work with exceptions  
- Reinforce operator overloading  
- Build a safe and functional custom string type  

---

## 🚀 How to Run
Compile with any C++17+ compiler:

g++ main.cpp  -o string_lab
./string_lab

---

## 📫 Contact
**Email:** muslimov.ramis13@gmail.com  
**Telegram:** https://t.me/muslimov_xvll

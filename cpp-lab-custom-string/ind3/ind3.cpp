#include <iostream>
#include <utility>     // std::swap
#include <stdexcept>   // std::out_of_range, std::exception
#include <cstddef>     // size_t
#include <clocale>     // setlocale

// Упрощённый класс String, работающий со C-style строками.
// Реализованы: конструкторы, деструктор, копирование, перемещение,
// присваивания, доступ по индексу с проверкой, reserve/push_back,
// операции +, +=, clear, сравнения и новая функция unique_chars_with.
class String {
private:
    char* data_;       // указатель на буфер символов (включая нуль-терминатор)
    size_t length_;    // длина строки (без нуль-терминатора)
    size_t capacity_;  // ёмкость буфера

    // Выделить буфер ёмкости cap (cap символов + 1 для '\0').
    // Может бросить std::bad_alloc при неудаче new.
    static char* allocate_buffer(size_t cap) {
        char* buf = new char[cap + 1]; // выделяем cap + 1 байт
        buf[0] = '\0';                 // делаем корректной пустую C-строку
        return buf;
    }

    // Вспомогательная функция: лексикографическое сравнение
    int compare_lex(const String& other) const {
        size_t i = 0;
        size_t n = (length_ < other.length_) ? length_ : other.length_;
        for (; i < n; ++i) {
            unsigned char a = static_cast<unsigned char>(data_[i]);
            unsigned char b = static_cast<unsigned char>(other.data_[i]);
            if (a != b) return (a < b) ? -1 : 1;
        }
        if (length_ == other.length_) return 0;
        return (length_ < other.length_) ? -1 : 1;
    }

public:
    // -------------------- Конструкторы / деструктор --------------------

    // Конструктор по умолчанию: пустая строка
    String()
        : data_(allocate_buffer(0)), length_(0), capacity_(0) {}

    // Конструктор из C-строки (const char*)
    String(const char* str)
        : data_(nullptr), length_(0), capacity_(0)
    {
        if (!str) { // nullptr трактуем как пустую строку
            data_ = allocate_buffer(0);
            length_ = 0;
            capacity_ = 0;
            return;
        }
        // вычисляем длину вручную (без <cstring>)
        const char* p = str;
        size_t len = 0;
        while (*p) { ++len; ++p; }

        char* buf = allocate_buffer(len); // может бросить
        for (size_t i = 0; i < len; ++i) buf[i] = str[i];
        buf[len] = '\0';

        data_ = buf;
        length_ = len;
        capacity_ = len;
    }

    // Копирующий конструктор (глубокое копирование)
    String(const String& other)
        : data_(nullptr), length_(other.length_), capacity_(other.capacity_)
    {
        char* buf = allocate_buffer(capacity_); // может бросить
        for (size_t i = 0; i < length_; ++i) buf[i] = other.data_[i];
        buf[length_] = '\0';
        data_ = buf;
    }

    // Перемещающий конструктор (без noexcept в этой реализации)
    String(String&& other)
        : data_(other.data_), length_(other.length_), capacity_(other.capacity_)
    {
        // Оставляем other в корректном пустом состоянии.
        // Здесь выделяется пустой буфер; это может бросить std::bad_alloc.
        other.data_ = allocate_buffer(0);
        other.length_ = 0;
        other.capacity_ = 0;
    }

    // Деструктор: освобождаем память
    ~String() {
        delete[] data_;
    }

    // -------------------- Доступ и операторы --------------------

    size_t length() const { return length_; }
    bool empty() const { return length_ == 0; }

    // operator[] с проверкой границ (бросает std::out_of_range)
    char& operator[](size_t index) {
        if (index >= length_) {
            throw std::out_of_range("String::operator[]: index out of range");
        }
        return data_[index];
    }
    const char& operator[](size_t index) const {
        if (index >= length_) {
            throw std::out_of_range("String::operator[] const: index out of range");
        }
        return data_[index];
    }

    const char* c_str() const { return data_; }

    // -------------------- swap и присваивания --------------------

    friend void swap(String& a, String& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.length_, b.length_);
        swap(a.capacity_, b.capacity_);
    }

    // copy-and-swap
    String& operator=(const String& other) {
        if (this != &other) {
            String tmp(other); // может бросить
            swap(*this, tmp);
        }
        return *this;
    }

    // move-assign (без noexcept)
    String& operator=(String&& other) {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            length_ = other.length_;
            capacity_ = other.capacity_;
            other.data_ = allocate_buffer(0);
            other.length_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // Присваивание из C-строки
    String& operator=(const char* str) {
        String tmp(str); // может бросить
        swap(*this, tmp);
        return *this;
    }

    // -------------------- Операции со строками --------------------

    // Конкатенация: возвращает новую строку, равную this + other
    String operator+(const String& other) const {
        size_t newlen = length_ + other.length_;
        char* buf = allocate_buffer(newlen); // может бросить
        for (size_t i = 0; i < length_; ++i) buf[i] = data_[i];
        for (size_t j = 0; j < other.length_; ++j) buf[length_ + j] = other.data_[j];
        buf[newlen] = '\0';
        String result;
        delete[] result.data_;
        result.data_ = buf;
        result.length_ = newlen;
        result.capacity_ = newlen;
        return result;
    }

    // this += other
    String& operator+=(const String& other) {
        size_t needed = length_ + other.length_;
        if (needed > capacity_) {
            size_t newcap = capacity_ == 0 ? 1 : capacity_ * 2;
            while (newcap < needed) newcap *= 2;
            reserve(newcap); // может бросить
        }
        for (size_t i = 0; i < other.length_; ++i) data_[length_ + i] = other.data_[i];
        length_ = needed;
        data_[length_] = '\0';
        return *this;
    }

    // this += C-строка
    String& operator+=(const char* str) {
        if (!str) return *this;
        const char* p = str;
        size_t len = 0;
        while (*p) { ++len; ++p; }
        size_t needed = length_ + len;
        if (needed > capacity_) {
            size_t newcap = capacity_ == 0 ? 1 : capacity_ * 2;
            while (newcap < needed) newcap *= 2;
            reserve(newcap); // может бросить
        }
        for (size_t i = 0; i < len; ++i) data_[length_ + i] = str[i];
        length_ = needed;
        data_[length_] = '\0';
        return *this;
    }

    // Очистить строку (сделать пустой)
    void clear() {
        length_ = 0;
        if (data_) data_[0] = '\0';
    }

    // -------------------- Сравнения --------------------

    bool operator==(const String& other) const {
        if (length_ != other.length_) return false;
        for (size_t i = 0; i < length_; ++i)
            if (data_[i] != other.data_[i]) return false;
        return true;
    }

    bool operator!=(const String& other) const { return !(*this == other); }
    bool operator<(const String& other) const { return compare_lex(other) < 0; }
    bool operator>(const String& other) const { return compare_lex(other) > 0; }

    // -------------------- Дополнительные методы --------------------

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;
        char* buf = allocate_buffer(new_cap); // может бросить
        for (size_t i = 0; i < length_; ++i) buf[i] = data_[i];
        buf[length_] = '\0';
        delete[] data_;
        data_ = buf;
        capacity_ = new_cap;
    }

    void push_back(char ch) {
        if (length_ + 1 > capacity_) {
            size_t newcap = capacity_ == 0 ? 1 : capacity_ * 2;
            if (newcap < length_ + 1) newcap = length_ + 1;
            reserve(newcap); // может бросить
        }
        data_[length_++] = ch;
        data_[length_] = '\0';
    }

    // -------------------- Новая функция: символы, не являющиеся общими --------------------
    // Формирует строку, содержащую все вхождения символов из *this и other,
    // которые НЕ встречаются в другой строке.
    // Учитываются только ASCII 0..127; порядок и количество вхождений сохраняются.
    String unique_chars_with(const String& other) const {
        // Массивы присутствия символов (ASCII 0..127)
        bool in_this[128] = { false };
        bool in_other[128] = { false };

        // Заполняем присутствие для this
        for (size_t i = 0; i < length_; ++i) {
            unsigned char uc = static_cast<unsigned char>(data_[i]);
            if (uc < 128) in_this[uc] = true;
        }
        // Заполняем присутствие для other
        for (size_t i = 0; i < other.length_; ++i) {
            unsigned char uc = static_cast<unsigned char>(other.data_[i]);
            if (uc < 128) in_other[uc] = true;
        }

        // Выделяем максимально возможный буфер (length_ + other.length_)
        size_t maxlen = length_ + other.length_;
        char* buf = allocate_buffer(maxlen); // может бросить

        size_t pos = 0;
        // Добавляем символы из this, которые не встречаются в other
        for (size_t i = 0; i < length_; ++i) {
            unsigned char uc = static_cast<unsigned char>(data_[i]);
            if (uc < 128 && !in_other[uc]) {
                buf[pos++] = data_[i];
            }
        }
        // Добавляем символы из other, которые не встречаются в this
        for (size_t i = 0; i < other.length_; ++i) {
            unsigned char uc = static_cast<unsigned char>(other.data_[i]);
            if (uc < 128 && !in_this[uc]) {
                buf[pos++] = other.data_[i];
            }
        }
        buf[pos] = '\0';

        // Формируем результат
        String result;
        delete[] result.data_;
        result.data_ = buf;
        result.length_ = pos;
        result.capacity_ = pos;
        return result;
    }
};

// -------------------- Тестирование в main -------------------
int main() {
    setlocale(LC_ALL, "ru");
    try {

        ////////////////////////////////////////////////////////////////////////////////////

        String s1;
        std::cout << "s1 empty: " << std::boolalpha << s1.empty() << ", len=" << s1.length() << '\n';

        String s2("Hello");
        std::cout << "s2: " << s2.c_str() << ", len=" << s2.length() << '\n';

        String s3 = s2;
        std::cout << "s3 (copy): " << s3.c_str() << '\n';

        s3[1] = 'a';
        std::cout << "s3 modified: " << s3.c_str() << '\n';
        std::cout << "s2 remains: " << s2.c_str() << '\n';

        // Перемещение через swap
        String tmp1;
        swap(tmp1, s3);
        String s4;
        swap(s4, tmp1);
        std::cout << "s4 (moved via swap): " << s4.c_str() << '\n';
        std::cout << "s3 after move via swap: " << s3.c_str() << '\n';

        // Присваивание из C-строки и push_back
        s4 = "New string";
        s4.push_back('!');
        std::cout << "s4 after push_back: " << s4.c_str() << '\n';

        // Операторы += и +
        String a("ABC");
        String b("DEF");
        a += b;
        std::cout << "a += b -> " << a.c_str() << '\n'; // ABCDEF

        a += "GHI";
        std::cout << "a += \"GHI\" -> " << a.c_str() << '\n'; // ABCDEFGHI

        String c = a + b; // конкатенация
        std::cout << "c = a + b -> " << c.c_str() << '\n';

        // clear
        c.clear();
        std::cout << "c after clear: \"" << c.c_str() << "\", len=" << c.length() << '\n';

        // сравнения
        String x("apple");
        String y("apricot");
        std::cout << "x == y: " << (x == y) << '\n';
        std::cout << "x != y: " << (x != y) << '\n';
        std::cout << "x < y: " << (x < y) << '\n';
        std::cout << "x > y: " << (x > y) << '\n';

        ////////////////////////////////////////////////////////////////////////////////////

        // Примеры использования
        String s5("abracadabra");
        String s6("barbar");

        std::cout << "s5: " << s5.c_str() << '\n';
        std::cout << "s6: " << s6.c_str() << '\n';

        // Формируем строку из символов, не общих для s5 и s6
        String uniq = s5.unique_chars_with(s6);
        std::cout << "Символы, не являющиеся общими (с сохранением вхождений): " << uniq.c_str() << '\n';
        // Ожидаемый вывод для примера: "cd" (вхождения 'c' и 'd' из s1; в s2 уникальных нет)

        // Дополнительный пример
        String a2("aaabx");
        String b2("bbbxy");
        String uniq2 = a2.unique_chars_with(b2);
        std::cout << "Пример 2: a2=\"" << a2.c_str() << "\", b2=\"" << b2.c_str() << "\" -> unique: \"" << uniq2.c_str() << "\"\n";
        // Ожидаемый результат: "aaay" (все 'a' из a2, т.к. 'a' не в b2; и 'y' из b2, т.к. 'y' не в a2)

        // Демонстрация проверки границ: намеренно вызвать исключение
        char ch = x[100]; // бросит std::out_of_range

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << '\n';
        return 1;
    }
    catch (...) {
        std::cerr << "Неизвестная ошибка. Программа завершена.\n";
        return 1;
    }
}

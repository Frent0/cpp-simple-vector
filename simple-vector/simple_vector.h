#pragma once

#include "array_ptr.h"

#include <iostream>
#include <cassert>
#include <initializer_list>
#include <stdexcept>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        : capacity_(capacity)
    {
    }

    size_t GetCapacity() {
        return capacity_;
    }

private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type())
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : values_(size), size_(size), capacity_(size)
    {
        std::fill(values_.Get(), values_.Get() + size, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : values_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), values_.Get());
    }

    SimpleVector(const SimpleVector& other):
        values_(other.capacity_),size_(other.size_) {
        std::copy(other.begin(),other.end(),values_.Get());
    }

    SimpleVector(SimpleVector&& other): values_(other.capacity_){
        swap(other);
    }

    explicit SimpleVector(ReserveProxyObj obj)
    {
        Reserve(obj.GetCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {

        if (&values_ != &rhs.values_) {

            ArrayPtr<Type> gap(rhs.GetCapacity());

            std::copy(rhs.begin(),rhs.end(),gap.Get());

            gap.swap(values_);

            size_ = rhs.size_;
            capacity_ = rhs.GetCapacity();

        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {

        if (this != &rhs) {

            values_.swap(rhs.values_);
            size_ = std::move(rhs.size_);
            capacity_ = std::move(rhs.size_);
        }
        return *this;
    }


    void Reserve(size_t new_capacity) {

        if (new_capacity < capacity_) {
            return;
        }

        if (new_capacity > capacity_) {

            ArrayPtr<Type> gap(new_capacity);

            std::copy(values_.Get(),values_.Get() + size_, gap.Get());

            gap.swap(values_);

            capacity_ = new_capacity;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {

        if (capacity_ == 0) {
            ArrayPtr<Type> gap(1);

            gap.swap(values_);

            values_[size_] = item;
            ++capacity_;
            ++size_;
            return;
        }

        if (size_ == capacity_) {
            ArrayPtr<Type> gap(capacity_ * 2);

            std::copy(values_.Get(),values_.Get() + size_, gap.Get());
            gap.swap(values_);

            values_[size_] = item;
            capacity_ *= 2;
            ++size_;
            return;
        }

        values_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {

        if (capacity_ == 0) {
            ArrayPtr<Type> gap(1);

            gap.swap(values_);

            values_[size_] = std::move(item);
            capacity_ = 1;
            ++size_;
            return;
        }

        if (size_ == capacity_) {
            ArrayPtr<Type> gap(capacity_ * 2);

            std::move(values_.Get(), values_.Get() + size_, gap.Get());
            gap.swap(values_);

            values_[size_] = std::move(item);
            capacity_ *= 2;
            ++size_;
            return;
        }

        values_[size_] =  std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        int distance_ = std::distance(cbegin(), pos);

        if (capacity_ == 0) {
            ArrayPtr<Type> gap(1);

            gap[0] = value;

            gap.swap(values_);

            ++size_;
            ++capacity_;

            return &values_[distance_];
        }

        if (size_ == capacity_) {
            ArrayPtr<Type> gap(capacity_ * 2);

            std::copy(values_.Get(), values_.Get() + distance_, gap.Get());
            std::copy_backward(values_.Get() + distance_, values_.Get() + size_, gap.Get() + size_ + 1);

            gap[distance_] = value;
            gap.swap(values_);

            ++size_;
            capacity_ *= 2;

            return &values_[distance_];
        }

        if (size_ < capacity_) {
            std::copy_backward(values_.Get() + distance_, values_.Get() + size_ ,values_.Get() + size_ + 1);

            values_[distance_] = value;

            ++size_;
        }

        return &values_[distance_];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        int distance_ = std::distance(cbegin(), pos);

        if (capacity_ == 0) {
            ArrayPtr<Type> gap(1);

            gap[0] = std::move(value);

            gap.swap(values_);

            ++size_;
            ++capacity_;

            return &values_[distance_];
        }

        if (size_ == capacity_) {
            ArrayPtr<Type> gap(capacity_ * 2);

            std::move(values_.Get(), values_.Get() + distance_, gap.Get());
            std::move_backward(values_.Get() + distance_, values_.Get() + size_, gap.Get() + size_ + 1);

            gap[distance_] = std::move(value);
            gap.swap(values_);

            ++size_;
            capacity_ *= 2;

            return &values_[distance_];
        }

        if (size_ < capacity_) {
            std::move_backward(values_.Get() + distance_, values_.Get() + size_, values_.Get() + size_ + 1);

            values_[distance_] = std::move(value);

            ++size_;
        }

        return &values_[distance_];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        int distance_ = std::distance(cbegin(), pos);

        std::move(values_.Get() + distance_ + 1, values_.Get() + size_, values_.Get() + distance_);

        --size_;

        return &values_[distance_];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        values_.swap(other.values_);
        std::swap(capacity_,other.capacity_);
        std::swap(size_, other.size_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return values_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return values_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) throw std::out_of_range("Out of range");
        return values_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) throw std::out_of_range("Out of range");
        return values_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    void Helper(Iterator begin, Iterator end) {
        for (; begin != end; ++begin) {
            *begin = std::move(Type());
        }
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }

        if (new_size <= capacity_) {
            Helper(values_.Get() + size_, values_.Get() + size_ + new_size);
        }

        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            Helper(temp.Get(), temp.Get() + new_capacity);
            std::move(values_.Get(), values_.Get() + capacity_, temp.Get());
            values_.swap(temp);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return values_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return values_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return values_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return values_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

private:
    ArrayPtr<Type> values_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(),lhs.end(),rhs.begin(),rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(),lhs.end(),rhs.begin(),rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

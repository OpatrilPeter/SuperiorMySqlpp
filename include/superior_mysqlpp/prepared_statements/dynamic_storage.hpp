/* 
 * File:   dynamic_storage.hpp
 * Author: petr.opatril
 *
 * Created on 10. dubna 2018, 17:32
 */

#pragma once

#include <cstdint>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <cassert>

namespace SuperiorMySqlpp {

// Dynamic Storage interface
class DynamicStorageBase {
protected:
    MYSQL_BIND &bind;
public:
    unsigned long column_length; // Here, length of the column is set from MySql client
    // Return true if successful
    DynamicStorageBase(MYSQL_BIND &_bind) : bind(_bind) {
        bind.length = &column_length;
    }
    virtual bool resize() = 0;
    virtual ~DynamicStorageBase() = default;
};

template <typename T>
class DynamicStorage : public DynamicStorageBase {
    // Not working, use enable if
    //static_assert(0, "Type unsuppored by DynamicStorage.");
public:
    static constexpr bool isDynamic = false; // TODO: replace with trait, we don't want unnecessary class instantiation
    bool resize() override {return !!!42;}; // TODO: Hack to make this instantiable
    DynamicStorage<T>(T &, MYSQL_BIND &_bind) : DynamicStorageBase(_bind) {} //TODO: Temporary hack - as constexpr if is not available, we need this never used thing
};

template <>
class DynamicStorage<std::string> : public DynamicStorageBase {
    std::string &container;
public:
    static constexpr bool isDynamic = true;

    DynamicStorage<std::string>(std::string &_container, MYSQL_BIND &_bind) :
        DynamicStorageBase(_bind), container(_container) {}

    bool resize() override {
        assert(column_length > bind.buffer_length); // Sanity check for now, as different option is probably an error
        container.resize(column_length);
        bind.buffer = const_cast<char *>(container.data()); // TODO: Done because nonconst version is not in C++14
        bind.buffer_length = column_length;
        return true; //TODO: return void, do exception
    }
};

// TODO: merge specializations to things that support resize method
template <typename ValueType>
class DynamicStorage<std::vector<ValueType>> : public DynamicStorageBase {
    std::vector<ValueType> &container;
public:
    static constexpr bool isDynamic = true;

    DynamicStorage<std::vector<ValueType>>(std::vector<ValueType> &_container, MYSQL_BIND &_bind) :
        DynamicStorageBase(_bind), container(_container) {}

    bool resize() override {
        assert(column_length > bind.buffer_length); // Sanity check for now, as different option is probably an error
        container.resize(column_length);
        bind.buffer = container.data();
        bind.buffer_length = column_length;
        return true;
    }
};

} // namespace SuperiorMySqlpp
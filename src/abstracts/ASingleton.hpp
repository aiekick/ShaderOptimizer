#pragma once
#include <memory>

#define IMPLEMENT_SINGLETON(TTYPE)                           \
public:                                                      \
    static std::unique_ptr<TTYPE>& initSingleton() {         \
        static auto mp_instance = std::make_unique<TTYPE>(); \
        return mp_instance;                                  \
    }                                                        \
    static TTYPE& ref() {                                    \
        return *initSingleton().get();                       \
    }                                                        \
    static void unitSingleton() {                            \
        initSingleton().reset();                             \
    }

#define IMPLEMENT_SHARED_SINGLETON(TTYPE)                    \
public:                                                      \
    static std::shared_ptr<TTYPE> initSingleton() {         \
        static auto mp_instance = std::make_shared<TTYPE>(); \
        return mp_instance;                                  \
    }                                                        \
    static std::shared_ptr<TTYPE> ref() {                   \
        return initSingleton();                             \
    }                                                        \
    static void unitSingleton() {                            \
        initSingleton().reset();                             \
    }

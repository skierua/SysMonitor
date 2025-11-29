#ifndef TEMPLIB_H
#define TEMPLIB_H

#include <QByteArray>

// currently not used

/*template <class Implementation>
class BaseMonitor {

public:
    int test() { return impl()->test(); }
    int crntEUID() { return impl()->crntEUID(); }

private:
    Implementation* impl() {  return static_cast<Implementation*>(this); }
}; */

template <typename Derived>
class StaticBase {
public:
    int test() { return impl()->test(); }

protected:
    // The key to making it "static" and non-instantiable:
    // Only derived classes (which are friends) can access constructors,
    // and we delete them anyway to be safe.
    StaticBase() = default;
    StaticBase(const StaticBase&) = delete;
    StaticBase(StaticBase&&) = delete;
    StaticBase& operator=(const StaticBase&) = delete;
    StaticBase& operator=(StaticBase&&) = delete;

private:
    Derived* impl() {  return static_cast<Derived*>(this); }
};

#endif // TEMPLIB_H

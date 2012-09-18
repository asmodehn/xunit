#ifndef TESTASSEMBLY_H_
#define TESTASSEMBLY_H_

#if defined(WIN32)
#include <Windows.h>
#endif

#include <string>
#include "xUnit++/ExportApi.h"

namespace xUnitpp
{

class TestAssembly
{
private:
    typedef bool (TestAssembly::*bool_type)() const;
    bool is_valid() const;

#if !defined(WIN32)
    typedef void * HMODULE;
#endif

public:
    TestAssembly(const std::string &file);
    ~TestAssembly();

    // !!!VS enable this when Visual Studio supports it
    //explicit operator bool() const
    //{
    //    return module != nullptr;
    //}

    operator bool_type() const;

    xUnitpp::EnumerateTestDetails EnumerateTestDetails;
    xUnitpp::FilteredTestsRunner FilteredTestsRunner;

private:
    std::string tempFile;
    HMODULE module;
};

}

#endif
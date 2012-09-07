#include <memory>
#include "xUnit++/xUnit++.h"

using xUnitpp::xUnitAssert;
using xUnitpp::Assert;

SUITE(AssertNull)
{

FACT(NullForRawPointerSuccess)
{
    Assert.Null(nullptr);
}

FACT(NullForDoubleConstRawPointerSuccess)
{
    const int * const px = nullptr;

    Assert.Null(px);
}

FACT(NullForSmartPointerSuccess)
{
    Assert.Null(std::unique_ptr<int>());
}

FACT(NullForRawPointerAssertsOnFailure)
{
    int x;
    Assert.Throws<xUnitAssert>([=]() { Assert.Null(&x); });
}

FACT(NullForSmartPointerAssertsOnFailure)
{
    std::unique_ptr<int> x(new int);
    Assert.Throws<xUnitAssert>([&]() { Assert.Null(x); });
}

FACT(NullAppendsUserMessage)
{
    static const std::string msg = "xUnit++";
    int x;
    auto assert = Assert.Throws<xUnitAssert>([=]() { Assert.Null(&x) << msg; });

    Assert.Contains(assert.what(), msg.c_str());
}

}
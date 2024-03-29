#include <sstream>
#include "xUnit++/xUnit++.h"

SUITE("ToString")
{

DATA_THEORY("ToString should not move parameters", (const std::string &s),
    []() -> std::vector<std::tuple<std::string>>
    {
        std::vector<std::tuple<std::string>> data;
        data.emplace_back("ABCD");
        return data;
    })
{
    Assert.Equal("ABCD", s);
}

FACT("ToString will print pointer addresses")
{
    int x;
    int y;

    auto result = Assert.Throws<xUnitpp::xUnitAssert>([&]() { Assert.Equal(&x, &y); });

    std::stringstream str;
    str << "int *: " << std::showbase << std::hex << reinterpret_cast<int>(&x);

    Assert.Equal(str.str(), result.Expected());
}

}
